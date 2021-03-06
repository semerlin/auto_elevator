/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "expand.h"
#include "stm32f10x_cfg.h"
#include "global.h"
#include "trace.h"
#include "parameter.h"
#include "protocol_expand.h"
#include "boardmap.h"
#include "config.h"
#include "dbgserial.h"


#undef __TRACE_MODULE
#define __TRACE_MODULE  "[EXPAND]"

typedef struct
{
    uint8_t len;
    uint8_t data[8];
} expand_data;

extern parameters_t board_parameter;

#ifdef __EXPAND
typedef enum
{
    REGISTER_SUCCESS,
    REGISTER_ID_EXISTS,
    REGISTER_FLOOR_EXISTS,
    REGISTER_FAIL,
} register_status_t;
static register_status_t register_status = REGISTER_FAIL;
#define REGISTER_INTERVAL         (2000 / portTICK_PERIOD_MS)
#endif

#define EXPAND_QUEUE_LEN   8
static xQueueHandle xExpandRecvQueue = NULL;
static xQueueHandle xExpandSendQueue = NULL;

#if DUMP_EXPAND
/**
 * @brief dump message
 * @param data - message to dump
 * @param len - data length
 */
static void dump_message(uint8_t dir, const uint8_t *data, uint8_t len)
{
    if (dir)
    {
        TRACE("send data: ");
    }
    else
    {
        TRACE("recv data: ");
    }
    for (int i = 0; i < len; ++i)
    {
        dbg_putchar("0123456789abcdef"[data[i] >> 4]);
        dbg_putchar("0123456789abcdef"[data[i] & 0x0f]);
        dbg_putchar(' ');
    }
    dbg_putchar('\r');
    dbg_putchar('\n');
}
#endif


#ifdef __EXPAND
/**
 * @brief board register status callback functin
 * @param[in] data: resgiter status data
 * @param[in] len: register status data length
 */
static void register_status_cb(uint8_t *data, uint8_t len)
{
    if (REGISTER_SUCCESS != register_status)
    {
        /** only process failed issue, in case of multi message send,
            first is success, second is faild */
        register_status = (register_status_t) * data;
    }
}
#endif

/**
 * @brief initialize can
 */
static void can_init(void)
{
    CAN_Config config;
    CAN_Filter filter;

    CAN_StructInit(&config);
    config.ttcm = FALSE;
    config.abom = FALSE;
    config.awum = FALSE;
    config.nart = FALSE;
    config.rflm = FALSE;
    config.txfp = FALSE;
#if LOOP_BACK_TEST
    config.mode = CAN_Mode_LoopBack;
#else
    config.mode = CAN_Mode_Normal;
#endif

#if USE_SPEED_100K
    /* 100k */
    config.sjw = CAN_SJW_1tq;
    config.bs1 = CAN_BS1_7tq;
    config.bs2 = CAN_BS2_2tq;
    config.prescaler = 36;
#else
    /** 20k */
    config.sjw = CAN_SJW_1tq;
    config.bs1 = CAN_BS1_14tq;
    config.bs2 = CAN_BS2_3tq;
    config.prescaler = 100;
#endif

    bool ret = CAN_Init(CAN1, &config);


    filter.number = 0;
    filter.mode = CAN_FilterMode_IdMask;
    filter.scale = CAN_FilterScale_32bit;
#ifdef __MASTER
    filter.id_high = 0;
    filter.id_low = 0;
    filter.mask_id_high = 0;
    filter.mask_id_low = 0;
#else
    filter.id_high = 0;
    filter.id_low = (((uint32_t)ID_BOARD_MASTER << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xffff;
    filter.mask_id_high = 0xffff;
    filter.mask_id_low = 0xffff;
#endif

    filter.fifo_assignment = CAN_Filter_FIFO0;
    filter.activation = TRUE;
    CAN_FilterInit(&filter);

    /* setup interrupt */
    NVIC_Config nvicConfig = {USB_LP_CAN_RX0_IRQChannel, CAN1_PRIORITY, 0, TRUE};
    NVIC_Init(&nvicConfig);
    /* receive interrupt */
    CAN_ITEnable(CAN1, CAN_IT_FMP0, TRUE);
}

/**
 * @brief send message
 * @param data - data to send
 * @param len - data length
 * @retval success sended length
 */
static uint8_t can_send_msg(const uint8_t *data, uint8_t len)
{
    uint8_t mbox;
    uint16_t count = 0;
    CAN_TxMsg msg;
    msg.std_id = 0;
#if LOOP_BACK_TEST
    msg.ext_id = ID_BOARD_MASTER;
#else
    msg.ext_id = board_parameter.id_board;
#endif
    msg.ide = CAN_ID_EXT;
    msg.rtr = CAN_RTR_DATA;
    msg.dlc = len;

    for (uint8_t i = 0; i < len; ++i)
    {
        msg.data[i] = data[i];
    }

    /** send message */
    mbox = CAN_Transmit(CAN1, &msg);
    if (CAN_TxStatus_NoMailBox == mbox)
    {
        return 0;
    }
    else
    {
        while ((CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1, mbox)) &&
               (count < 0xfff))
        {
            /** wait message send finish */
            count++;
        }

        if (count > 0xfff)
        {
            return 0;
        }

        return len;
    }
}

#ifdef __EXPAND
/**
 * @brief can receive message task
 * @param pvParameters - task parameter
 */
static void vRegisterBoard(void *pvParameters)
{
    if (REGISTER_SUCCESS != register_status)
    {
        register_board(board_parameter.id_board, board_parameter.start_floor);
    }
}
#endif

/**
 * @brief can receive message task
 * @param pvParameters - task parameter
 */
static void vExpandRecv(void *pvParameters)
{
    expand_data data;
    for (;;)
    {
        if (xQueueReceive(xExpandRecvQueue, &data, portMAX_DELAY))
        {
            process_expand_data(data.data, data.len);
#if DUMP_EXPAND
            dump_message(0, data.data, data.len);
#endif
        }
    }
}

/**
 * @brief can send message task
 * @param pvParameters - task parameter
 */
static void vExpandSend(void *pvParameters)
{
    expand_data data;
    for (;;)
    {
        if (xQueueReceive(xExpandSendQueue, &data, portMAX_DELAY))
        {
            can_send_msg(data.data, data.len);
#if DUMP_EXPAND
            dump_message(1, data.data, data.len);
#endif
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief send data to can
 * @param data - data to send
 * @param len - data length
 */
void expand_send_data(const uint8_t *buf, uint8_t len)
{
    expand_data data;
    data.len = len;
    if (data.len > 8)
    {
        data.len = 8;
    }
    memcpy(data.data, buf, data.len);
    xQueueSend(xExpandSendQueue, &data, 50 / portTICK_PERIOD_MS);
}

/**
 * @brief send data to can imeediately
 * @param data - data to send
 * @param len - data length
 */
void expand_send_data_immediately(const uint8_t *buf, uint8_t len)
{
    can_send_msg(buf, len);
#if DUMP_EXPAND
    dump_message(1, buf, len);
#endif
}

#ifdef __EXPAND
/**
 * @brief get board register status
 * @retval TRUE: board has been registered successfully
 * @retval FALSE: board has not beed registered
 */
bool is_expand_board_registered(void)
{
    return (REGISTER_SUCCESS == register_status);
}
#endif

/**
 * @brief expand initialize
 * @return initialize status
 */
bool expand_init(void)
{
    TRACE("initialize expand module...\r\n");
    can_init();
    xExpandRecvQueue = xQueueCreate(EXPAND_QUEUE_LEN, sizeof(expand_data));
    xExpandSendQueue = xQueueCreate(EXPAND_QUEUE_LEN, sizeof(expand_data));
    xTaskCreate(vExpandRecv, "expand_recv", EXPAND_STACK_SIZE, NULL,
                EXPAND_PRIORITY, NULL);
    xTaskCreate(vExpandSend, "expand_send", EXPAND_STACK_SIZE, NULL,
                EXPAND_PRIORITY, NULL);
#ifdef __EXPAND
    set_register_cb(register_status_cb);
    TimerHandle_t expand_tmr = xTimerCreate("expand_tmr", REGISTER_INTERVAL, TRUE, NULL,
                                            vRegisterBoard);
    if (NULL == expand_tmr)
    {
        TRACE("initialise expand module failed!\r\n");
        return FALSE;
    }
    xTimerStart(expand_tmr, 0);
#endif
    return TRUE;
}

/**
 * @brief can rx interrupt
 */
void USB_LP_CAN_RX0_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    CAN_RxMsg rx_msg;
    expand_data data;

    /* receive data */
    CAN_Receive(CAN1, CAN_FIFO0, &rx_msg);
    data.len = rx_msg.dlc;
    memcpy(data.data, rx_msg.data, rx_msg.dlc);
    xQueueSendFromISR(xExpandRecvQueue, &data, &xHigherPriorityTaskWoken);

    /* check if there is any higher priority task need to wakeup */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
