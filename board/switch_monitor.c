/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "switch_monitor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "global.h"
#include "pinconfig.h"
#include "elevator.h"
#include "led_status.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[switchmtl]"


/* switch 0->1 means arrive，1->0 means leave */
#define SWITCH_MONITOR_INTERVAL    (8 / portTICK_PERIOD_MS)
#define UPPER_SWITCH     "SWITCH1"
#define LOWER_SWITCH     "SWITCH2"

static switch_status cur_status = switch_arrive;

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
    
    for (int i = 0; i < 8; ++i)
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
 * @brief switch monitor task
 * @param pvParameters - task parameter
 */
static void vSwitchMonitor(void *pvParameters)
{
    uint8_t switch_prev = switch_val();
    uint8_t switch_cur = switch_prev;
    for (;;)
    {
        switch_cur = filter_switch_val();
        if (0 != switch_cur)
        {
            if (0x03 == switch_cur)
            {
                cur_status = switch_arrive;
            }
            else
            {
                cur_status = switch_run;
            }
            
            if (switch_cur != switch_prev)
            {
                /* elevator state changed */
                if ((0x01 == switch_prev) && 
                    ((0x03 == switch_cur) || (0x02 == switch_cur)))
                {
                    elev_increase();
                }
                else if ((0x02 == switch_prev) && 
                         ((0x03 == switch_cur) || (0x01 == switch_cur)))
                {
                    elev_decrease();
                }

                switch_prev = switch_cur;
            }
        }
        vTaskDelay(SWITCH_MONITOR_INTERVAL);
    }
}

/**
 * @brief initialize switch monitor
 * @return init status
 */
bool switch_monitor_init(void)
{
    TRACE("initialize switch monitor...\r\n");
    xTaskCreate(vSwitchMonitor, "switchmonitor", SWITCH_MONITOR_STACK_SIZE, NULL, 
                    SWITCH_MONITOR_PRIORITY, NULL);
    return TRUE;
}
