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
#define ALTIMETER_PRIORITY           (tskIDLE_PRIORITY + 4)
#define ALTIMETER_CALC_PRIORITY      (tskIDLE_PRIORITY + 2)
#define BLUETOOTH_PRIORITY           (tskIDLE_PRIORITY + 4)
#define LED_PROCESS_PRIORITY         (tskIDLE_PRIORITY + 3)
#endif
#define PROTOCOL_PRIORITY            (tskIDLE_PRIORITY + 4)
#define ELEV_PRIORITY                (tskIDLE_PRIORITY + 1)
#define EXPAND_PRIORITY              (tskIDLE_PRIORITY + 2)

/* task stack definition */
#ifdef __MASTER
#define SWITCH_MONITOR_STACK_SIZE    (configMINIMAL_STACK_SIZE)
#define ALTIMETER_STACK_SIZE         (configMINIMAL_STACK_SIZE * 2)
#define ALTIMETER_CALC_STACK_SIZE    (configMINIMAL_STACK_SIZE)
#define BLUETOOTH_STACK_SIZE         (configMINIMAL_STACK_SIZE * 2)
#define LED_PROCESS_STACK_SIZE       (configMINIMAL_STACK_SIZE)
#endif
#define PROTOCOL_STACK_SIZE          (configMINIMAL_STACK_SIZE * 2)
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
