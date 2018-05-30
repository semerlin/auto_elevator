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

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[app]"

#define VERSION  ("v0.0.0.1_alpha")


/**
 * @brief initialize system
 * @param pvParameters - task parameter
 */
static void vInitSystem(void *pvParameters)
{
    TRACE("startup application...\r\n");
    TRACE("version = %s\r\n", VERSION);
    
    vTaskDelete(NULL);
}

/**
 * @brief start system
 */
void ApplicationStartup()
{
    license_init();
    xTaskCreate(vInitSystem, "Init", INIT_SYSTEM_STACK_SIZE, NULL, 
                    INIT_SYSTEM_PRIORITY, NULL);
    
    /* Start the scheduler. */
    vTaskStartScheduler();
}
