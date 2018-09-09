/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "switch_monitor.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "trace.h"
#include "global.h"
#include "pinconfig.h"
#include "elevator.h"
#include "led_status.h"
#include "stm32f10x_cfg.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[switchmtl]"


/* switch 0->1 means arrive，1->0 means leave */
#define UPPER_SWITCH     "SWITCH1"
#define LOWER_SWITCH     "SWITCH2"

static xQueueHandle xSwitchVals = NULL;
static switch_status cur_status = switch_arrive;

static uint8_t filter_step = 0;
static uint8_t filter_prev = 0;
static uint8_t filter_cur = 0;

/**
 * @brief get switch value
 */
static uint8_t switch_val(void)
{
    uint8_t val = 0;
    if (is_pinset(UPPER_SWITCH))
    {
        val |= 0x01;
        val <<= 1;
    }

    if (is_pinset(LOWER_SWITCH))
    {
        val |= 0x01;
    }

    return val;
}

/**
 * @brief get switch current status
 * @return current switch status
 */
switch_status switch_get_status(void)
{
    return cur_status;
}

/**
 * @brief filter switch value
 * @return switch value
 */
uint8_t filter_switch_val(void)
{
    uint8_t switch_map[4] = {0x00, 0x01, 0x02, 0x03};
    uint8_t switch_cnt[4] = {0, 0, 0, 0};
    uint8_t max = 0;

    for (int i = 0; i < 6; ++i)
    {
        switch_cnt[switch_val()] += 1;
    }

    for (int i = 0; i < 4; ++i)
    {
        if (max < switch_cnt[i])
        {
            max = switch_cnt[i];
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if (switch_cnt[i] == max)
        {
            return switch_map[i];
        }
    }

    return 0;
}

/**
 * @brief filter interrupt handler
 */
void TIM2_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    switch (filter_step)
    {
    case 0:
        filter_cur = filter_switch_val();
        filter_prev = filter_cur;
        filter_step = 1;
        break;
    case 1:
        filter_cur = filter_switch_val();
        if (filter_prev == filter_cur)
        {
            xQueueSendFromISR(xSwitchVals, &filter_cur,
                              &xHigherPriorityTaskWoken);
        }
        filter_step = 0;
        break;
    default:
        break;
    }

    TIM_ClearIntFlag(TIM2, TIM_INT_FLAG_UPDATE);

    /* check if there is any higher priority task need to wakeup */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}


/**
 * @brief switch monitor task
 * @param pvParameters - task parameter
 */
static void vSwitchMonitor(void *pvParameters)
{
    /**
     * 0-1-3: floor increase
     * 0-2-3: floor decrese
     */
    uint8_t switch_prev = switch_val();
    uint8_t switch_cur = switch_prev;
    bool detect_switch_zero = FALSE;
    if (0 == switch_prev)
    {
        detect_switch_zero = TRUE;
    }
    for (;;)
    {
        if (xQueueReceive(xSwitchVals, &switch_cur, portMAX_DELAY))
        {
            if (detect_switch_zero)
            {
                if (0 != switch_cur)
                {
                    if (switch_cur != switch_prev)
                    {
                        TRACE("cur = %d, prev = %d\r\n", switch_cur, switch_prev);
                        /* elevator state changed */
                        if ((0x01 == switch_prev) && (0x03 == switch_cur))
                        {
                            elev_increase();
                            detect_switch_zero = FALSE;
                        }
                        else if ((0x02 == switch_prev) && (0x03 == switch_cur))
                        {
                            elev_decrease();
                            detect_switch_zero = FALSE;
                        }

                        switch_prev = switch_cur;
                    }
                }
            }
            else
            {
                if (0 == switch_cur)
                {
                    TRACE("detect zero\r\n");
                    switch_cur = 0x03;
                    switch_prev = 0x03;
                    detect_switch_zero = TRUE;
                }
            }
        }
    }
}

/**
 * @brief init switch filter
 */
void init_filter(void)
{
    /** timer count interval 100us */
    TIM_SetCntInterval(TIM2, 100);
    /** timer reload value 80(5ms) */
    TIM_SetAutoReload(TIM2, 65);
    TIM_SetCountMode(TIM2, TIM_COUNTMODE_UP);
    TIM_IntEnable(TIM2, TIM_INT_UPDATE, TRUE);

    /* setup interrupt */
    NVIC_Config nvicConfig = {TIM2_IRQChannel, TIM2_PRIORITY, 0, TRUE};
    NVIC_Init(&nvicConfig);
    TIM_Enable(TIM2, TRUE);
}
/**
 * @brief initialize switch monitor
 * @return init status
 */
bool switch_monitor_init(void)
{
    TRACE("initialize switch monitor...\r\n");
    xSwitchVals = xQueueCreate(10, (UBaseType_t)sizeof(portCHAR));
    xTaskCreate(vSwitchMonitor, "switchmonitor", SWITCH_MONITOR_STACK_SIZE, NULL,
                SWITCH_MONITOR_PRIORITY, NULL);
    init_filter();
    return TRUE;
}
