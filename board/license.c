/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "FreeRTOS.h"
#include "timers.h"
#include "global.h"
#include "trace.h"
#include "serial.h"
#include "parameter.h"
#include "stm32f10x_cfg.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[license]"


#define LICENSE_MONITOR_INTERVAL     (1000 / portTICK_PERIOD_MS)

/**
 * @brief license check task
 * @param pvParameter - task parameter
 */
static void vLicense(void *pvParameters)
{
    static uint32_t g_count = 0;
    g_count ++;
    if (g_count >= 3600 * 24 * 30)
    {
        /* license expired */
        TRACE("license expired!\r\n");
        reset_param();
        SCB_SystemReset();
    }
}

/**
 * @brief init license check
 */
bool license_init(void)
{
    TRACE("initialise license system...\r\n");
    TimerHandle_t license_tmr = xTimerCreate("license_tmr", LICENSE_MONITOR_INTERVAL, TRUE, NULL,
                                             vLicense);
    if (NULL == license_tmr)
    {
        TRACE("initialise license system failed!\r\n");
        return FALSE;
    }
    xTimerStart(license_tmr, 0);
    return TRUE;
}
