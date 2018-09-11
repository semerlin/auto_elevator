/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "FreeRTOS.h"

/* task priority definition */
#ifdef __MASTER
#define SWITCH_MONITOR_PRIORITY      (tskIDLE_PRIORITY + 3)
#define ROBOT_PRIORITY               (tskIDLE_PRIORITY + 1)
#define ALTIMETER_PRIORITY           (tskIDLE_PRIORITY + 4)
#define ALTIMETER_CALC_PRIORITY      (tskIDLE_PRIORITY + 3)
#endif
#define LICENSE_PRIORITY             (tskIDLE_PRIORITY + 1)
#define LED_MONITOR_PRIORITY         (tskIDLE_PRIORITY + 2)
#define PROTOCOL_PRIORITY            (tskIDLE_PRIORITY + 4)
#define ELEV_PRIORITY                (tskIDLE_PRIORITY + 1)
#define EXPAND_PRIORITY              (tskIDLE_PRIORITY + 2)

/* task stack definition */
#ifdef __MASTER
#define SWITCH_MONITOR_STACK_SIZE    (configMINIMAL_STACK_SIZE)
#define ROBOT_STACK_SIZE             (configMINIMAL_STACK_SIZE)
#define ALTIMETER_STACK_SIZE         (configMINIMAL_STACK_SIZE * 2)
#define ALTIMETER_CALC_STACK_SIZE    (configMINIMAL_STACK_SIZE)
#endif
#define LICENSE_STACK_SIZE           (configMINIMAL_STACK_SIZE)
#define LED_MONITOR_STACK_SIZE       (configMINIMAL_STACK_SIZE)
#define PROTOCOL_STACK_SIZE          (configMINIMAL_STACK_SIZE)
#define ELEV_STACK_SIZE              (configMINIMAL_STACK_SIZE)
#define EXPAND_STACK_SIZE            (configMINIMAL_STACK_SIZE)

/* interrupt priority */
#define CAN1_PRIORITY          (11)
#define USART1_PRIORITY        (12)
#define TIM2_PRIORITY          (9)

#ifdef __MASTER
#define ARRIVE_JUDGE    (0)
#endif

#endif /* _GLOBAL_H_ */

