/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include <string.h>
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
#include "bluetooth.h"
#endif
#include "led_status.h"
#include "expand.h"
#include "diagnosis.h"
#include "dbgserial.h"


#undef __TRACE_MODULE
#define __TRACE_MODULE  "[app]"

#ifdef __MASTER
#define VERSION  ("v1.1.4.12")
#else
#define VERSION  ("v1.1.1.0")
#endif

parameters_t board_parameter;
uint8_t chip_id[12];

/**
 * @brief start system
 */
void ApplicationStartup()
{
#ifdef __MASTER
    TRACE("startup master application...\r\n");
    TRACE("Copyright 2018, Huang Yang <elious.huang@gmail.com>\r\n");
#else
    TRACE("startup expand application...\r\n");
    TRACE("Copyright 2018, Huang Yang <elious.huang@gmail.com>\r\n");
#endif
    TRACE("version = %s\r\n", VERSION);
    Get_ChipID(chip_id, NULL);
    TRACE("serial number = ");
    for (uint8_t i = 0; i < 12; ++i)
    {
        dbg_putchar("0123456789abcdef"[chip_id[i] >> 4]);
        dbg_putchar("0123456789abcdef"[chip_id[i] & 0x0f]);
    }
    dbg_putchar('\r');
    dbg_putchar('\n');

    diagnosis_init();
    license_init();
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

        if ((0xff == board_parameter.bt_name[0]) ||
            ('\0' == board_parameter.bt_name[0]))
        {
            strcpy((char *)board_parameter.bt_name, "HC-08");
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
        bt_init();
#endif
        led_monitor_init();
        elev_init();
        expand_init();
    }

    /* Start the scheduler. */
    vTaskStartScheduler();
}
