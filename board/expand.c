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
#include "expand.h"
#include "stm32f10x_cfg.h"
#include "global.h"
#include "trace.h"
#include "parameter.h"
#include "elevator.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[EXPAND]"

typedef struct
{
    uint8_t len;
    uint8_t data[8];
} expand_data;

#define MASTER_ID   0x01
#define SLAVE_ID    0x02

static uint8_t filter_id = MASTER_ID;
static uint8_t self_id = SLAVE_ID;

#define EXPAND_QUEUE_LEN   10
static xQueueHandle xExpandRecvQueue;
static xQueueHandle xExpandSendQueue;

static void process_elev_go(const uint8_t *data, uint8_t len);

/* process handle */
typedef struct
{
    uint8_t cmd;
    void (*process)(const uint8_t *data, uint8_t len);
} cmd_handle;

/* protocol command */
#define CMD_ELEV_GO            1
#define CMD_ELEV_LED           2

static cmd_handle cmd_handles[] =
{
    {CMD_ELEV_GO, process_elev_go},
};

/**
 * @brief init can
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
    config.mode = CAN_Mode_Normal;

    /* 100k */
    config.sjw = CAN_SJW_1tq;
    config.bs1 = CAN_BS1_3tq;
    config.bs2 = CAN_BS2_2tq;
    config.prescaler = 60;

    CAN_Init(CAN1, &config);

    filter.number = 1;
    filter.mode = CAN_FilterMode_IdMask;
    filter.scale = CAN_FilterScale_32bit;
    filter.id_high = 0;
    filter.id_low = (((uint32_t)filter_id << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xFFFF;
    filter.mask_id_high = 0xFFFF;
    filter.mask_id_low = 0xFFFF;
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
static uint8_t can_send_msg(uint8_t *data, uint8_t len)
{
    uint8_t mbox;
    uint16_t count = 0;
    CAN_TxMsg msg;
    msg.std_id = 0;
    msg.ext_id = self_id;
    msg.ide = CAN_ID_EXT;
    msg.rtr = CAN_RTR_DATA;
    msg.dlc = len;

    for (uint8_t i = 0; i < len; ++i)
    {
        msg.data[i] = data[i];
    }

    //发送数据
    mbox = CAN_Transmit(CAN1, &msg);
    while ((CAN_TxStatus_Failed == CAN_TransmitStatus(CAN1, mbox)) &&
           (count < 0xfff))
    {
        count++;    //等待发送结束
    }

    if (count > 0xfff)
    {
        return 0;
    }

    return len;
}

/**
 * @brief switch monitor task
 * @param pvParameters - task parameter
 */
static void vExpandRecv(void *pvParameters)
{
    expand_data data;
    for (;;)
    {
        if (xQueueReceive(xExpandRecvQueue, &data, portMAX_DELAY))
        {
            for (int i = 0; i < sizeof(cmd_handles) / sizeof(cmd_handles[0]); ++i)
            {
                if (data.data[0] == cmd_handles[i].cmd)
                {
                    cmd_handles[i].process(data.data + 1, data.len - 1);
                    break;
                }
            }
        }
    }
}

/**
 * @brief switch monitor task
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
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
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
    xQueueReceive(xExpandSendQueue, &data, 100 / portTICK_PERIOD_MS);
}

/**
 * @brief expand init
 * @return init status
 */
bool expand_init(void)
{
    TRACE("initialize expand module...\r\n");
#if 0
    if (board_master == param_get_board_type())
    {
        filter_id = SLAVE_ID;
        self_id = MASTER_ID;
    }
    else
    {
        filter_id = MASTER_ID;
        self_id = SLAVE_ID;
    }
#endif
    can_init();
    xExpandRecvQueue = xQueueCreate(EXPAND_QUEUE_LEN, sizeof(expand_data));
    xExpandSendQueue = xQueueCreate(EXPAND_QUEUE_LEN, sizeof(expand_data));
    xTaskCreate(vExpandRecv, "expand_recv", EXPAND_STACK_SIZE, NULL,
                EXPAND_PRIORITY, NULL);
    xTaskCreate(vExpandSend, "expand_send", EXPAND_STACK_SIZE, NULL,
                EXPAND_PRIORITY, NULL);
    return TRUE;
}

/**
 * @brief process elevator goto floor message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_go(const uint8_t *data, uint8_t len)
{
    char floor = (char)(*data);
    elev_go(floor);
}

/**
 * @brief notify led status changed
 * @param led_status - led status
 */
void expand_notify_led_change(uint8_t led_status)
{

}

void expand_elev_go(char floor)
{
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