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
#include "ledstatus.h"
#include "keymap.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[switchmtl]"


/* switch 0->1 means arrive，1->0 means leave */

#define SWITCH_MONITOR_INTERVAL    (100 / portTICK_PERIOD_MS)
#define UPPER_SWITCH     "SWITCH1"
#define LOWER_SWITCH     "SWITCH2"

/* switch value */
static uint8_t switch_prev = 0;
static uint8_t switch_cur = 0;

/**
 * @brief get switch value
 */
static uint8_t switch_val(void)
{
    uint8_t val = 0;
    if (is_pin_set(UPPER_SWITCH))
    {
        val |= 0x01;
        val <<= 1;
    }

    if (is_pin_set(LOWER_SWITCH))
    {
        val |= 0x01;
    }
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
        switch_cur = switch_val();
        if (switch_cur != switch_prev)
        {
            /* elevator state changed */
            if ((0x01 == switch_prev) && (0x03 == switch_cur))
            {
                floor_increase();
            }
            else if ((0x02 == switch_prev) && (0x03 == switch_cur))
            {
                floor_decrease();
            }

            switch_prev = switch_cur;
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
    xTaskCreate(vSwitchMonitor, "switchmonitor", SWITCH_MONITOR_STACK_SIZE, NULL, 
                    SWITCH_MONITOR_PRIORITY, NULL);

    return TRUE;
}
