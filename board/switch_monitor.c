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

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[switchmtl]"


/* switch 0->1 means arrive，1->0 means leave */

#define SWITCH_MONITOR_INTERVAL    (100 / portTICK_PERIOD_MS)
#define UPPER_SWITCH     "SWITCH1"
#define LOWER_SWITCH     "SWITCH2"

#if 0
static xSemaphoreHandle xMotorWorking = NULL;

/**
 * motor working detect interrupt handler
 */
void EXTI3_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t pin_num = 0;
    get_pininfo(MOTOR_DET_PIN_NAME, NULL, &pin_num);
    xSemaphoreGiveFromISR(xMotorWorking, &xHigherPriorityTaskWoken);
    /* check if there is any higher priority task need to wakeup */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    EXTI_ClrPending(pin_num);
}
#endif

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
                elev_increase();
            }
            else if ((0x02 == switch_prev) && (0x03 == switch_cur))
            {
                elev_decrease();
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

#if 0
    /* set pin interrupt */
    uint8_t pin_group = 0, pin_num = 0;
    get_pininfo(MOTOR_DET_PIN_NAME, &pin_group, &pin_num);
    EXTI_ClrPending(pin_num);
    GPIO_EXTIConfig((GPIO_Group)pin_group, pin_num);
    EXTI_SetTrigger(pin_num, Trigger_Rising);
    NVIC_Config nvicConfig = {EXTI3_IRQChannel, USART1_PRIORITY, 0, TRUE};
    NVIC_Init(&nvicConfig);
    EXTI_EnableLine_INT(pin_num, TRUE);
#endif
    return TRUE;
}
