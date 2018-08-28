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
#include "boardmap.h"
#include "keyctl.h"
#include "robot.h"
#include "led_monitor.h"
#include "protocol.h"
#include "elevator.h"
#ifdef __MASTER
#include "switch_monitor.h"
#include "altimeter.h"
#include "altimeter_calc.h"
#include "floormap.h"
#endif
#include "led_status.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[app]"

#define VERSION  ("v1.1.0.0")

parameters_t board_parameter;

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

    ptl_init();

    if (is_param_setted())
    {
        board_parameter = param_get();
        boardmap_add(board_parameter.id_board, START_KEY, board_parameter.start_floor,
                     MAX_FLOOR_NUM, 0);
#ifdef __MASTER
        floormap_update();
        if ((0 == board_parameter.floor_height) ||
            (0xff == board_parameter.floor_height))
        {
            board_parameter.floor_height = 2600;
        }
#endif
        keyctl_init();
#ifdef __MASTER
        robot_init();
        if (CALC_PWD == board_parameter.calc_type)
        {
            switch_monitor_init();
        }
        else
        {
            altimeter_init();
            altimeter_calc_init();
        }
#endif
        led_monitor_init();
        elev_init();
    }

    /* Start the scheduler. */
    vTaskStartScheduler();
}
