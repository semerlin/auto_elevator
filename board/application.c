/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "stm32f10x_cfg.h"
#include "global.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "license.h"
#include "parameter.h"
#include "keymap.h"
#include "keyctl.h"
#include "robot.h"
#include "led_monitor.h"
#include "protocol.h"
#include "elevator.h"
#include "switch_monitor.h"
#include "led_status.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[app]"

#define VERSION  ("v0.0.0.1_alpha")


/**
 * @brief start system
 */
void ApplicationStartup()
{
    TRACE("startup application...\r\n");
    TRACE("version = %s\r\n", VERSION);
    //license_init();
    if (!param_init())
    {
        TRACE("startup application failed!\r\n");
    }
    
    protocol_init();
    
    if (is_param_setted())
    {
        keymap_init();
        keyctl_init();
        robot_init();
        switch_monitor_init();
        led_monitor_init();
        elev_init();
    }

    /* Start the scheduler. */
    vTaskStartScheduler();
}
