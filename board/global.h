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
#define LICENSE_PRIORITY             (tskIDLE_PRIORITY + 1)
#define LED_MONITOR_PRIORITY         (tskIDLE_PRIORITY + 2)
#define SWITCH_MONITOR_PRIORITY      (tskIDLE_PRIORITY + 3)
#define PROTOCOL_PRIORITY            (tskIDLE_PRIORITY + 4)
#define ELEV_PRIORITY                (tskIDLE_PRIORITY + 1)
#define ROBOT_PRIORITY               (tskIDLE_PRIORITY + 1)
#define EXPAND_PRIORITY              (tskIDLE_PRIORITY + 2)

/* task stack definition */
#define LICENSE_STACK_SIZE           (configMINIMAL_STACK_SIZE)
#define LED_MONITOR_STACK_SIZE       (configMINIMAL_STACK_SIZE)
#define SWITCH_MONITOR_STACK_SIZE    (configMINIMAL_STACK_SIZE)
#define PROTOCOL_STACK_SIZE          (configMINIMAL_STACK_SIZE)
#define ELEV_STACK_SIZE              (configMINIMAL_STACK_SIZE)
#ifdef __MASTER
#define ROBOT_STACK_SIZE             (configMINIMAL_STACK_SIZE)
#endif
#define EXPAND_STACK_SIZE            (configMINIMAL_STACK_SIZE)

/* interrupt priority */
#define CAN1_PRIORITY          (12)
#define USART1_PRIORITY        (13)
#define TIM2_PRIORITY          (14)

#ifdef __MASTER
#define ARRIVE_JUDGE    (0)
#endif

#endif /* _GLOBAL_H_ */

