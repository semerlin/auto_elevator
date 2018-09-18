/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifdef __MASTER
#include <string.h>
#include "altimeter_calc.h"
#include "altimeter.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include "trace.h"
#include "serial.h"
#include "global.h"
#include "dbgserial.h"
#include "stm32f10x_cfg.h"
#include "parameter.h"
#include "protocol_param.h"
#include "elevator.h"
#include "floormap.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[altimeter_calc]"

extern parameters_t board_parameter;
static calc_action_t calc_action = CALC_STOP;
static xQueueHandle xQueueCalc = NULL;
#define ALTIMETER_IDLE_INTERVAL    (200 / portTICK_PERIOD_MS)


/**
 * @brief update specified floor height
 * @param[in] floor: floor number
 * @param[in] height: floor height
 */
static uint16_t update_floor_height(uint8_t floor, uint32_t height)
{
    uint16_t ret = 0;
    for (uint8_t i = 0; i < MAX_FLOOR_NUM + MAX_EXPAND_FLOOR_NUM * (MAX_BOARD_NUM - 1); ++i)
    {
        if (board_parameter.floor_height[i].floor == floor)
        {
            if (board_parameter.floor_height[i].height > 0)
            {
                board_parameter.floor_height[i].height += (height / 10);
                board_parameter.floor_height[i].height /= 2;
            }
            else
            {
                board_parameter.floor_height[i].height = height / 10;
            }
            ret = board_parameter.floor_height[i].height;
            TRACE("update floor height: %d-%d(cm)", floor, board_parameter.floor_height[i].height);
        }
    }

    return ret;
}

/**
 * @brief altimeter receive distance task
 * @param pvParameters - task parameter
 */
static void vAltimeterCalc(void *pvParameters)
{
    uint8_t floor = 1;
    uint16_t average_height = 0;
    uint32_t distance = 0;
    for (;;)
    {
        if (CALC_START == calc_action)
        {
            if (xQueueReceive(xQueueCalc, &floor, portMAX_DELAY))
            {
                distance = altimeter_get_distance();
                average_height = update_floor_height(floor, distance);

                notify_calc(floor, average_height, distance / 10);
            }
        }
        else
        {
            vTaskDelay(ALTIMETER_IDLE_INTERVAL);
        }
    }
}

/**
 * @brief initialize altimeter
 */
bool altimeter_calc_init(void)
{
    TRACE("initialize altimeter calculate....\r\n");
    xQueueCalc = xQueueCreate(1, 1);
    xTaskCreate(vAltimeterCalc, "altimeter_calc", ALTIMETER_CALC_STACK_SIZE,
                NULL, ALTIMETER_CALC_PRIORITY, NULL);
    return TRUE;
}

void altimeter_calc_once(uint8_t floor)
{
    TRACE("calculate once: %d\r\n", floor);
    xQueueOverwrite(xQueueCalc, &floor);
}

bool altimeter_calc_run(calc_action_t action)
{
    TRACE("set calculate %d\r\n", action);
    calc_action = action;
    if (CALC_STOP == action)
    {
        param_store_floor_height(MAX_FLOOR_NUM + MAX_EXPAND_FLOOR_NUM * (MAX_BOARD_NUM - 1),
                                 board_parameter.floor_height);

        TRACE("rebooting...\r\n");
        SCB_SystemReset();
    }
    else
    {
        uint16_t average_height = 0;
        uint32_t distance = 0;
        distance = altimeter_get_distance();
        average_height = update_floor_height(1, distance);
        notify_calc(1, average_height, distance / 10);
        elev_go(2);
    }
    return TRUE;
}

bool altimeter_is_calculating(void)
{
    return (CALC_START == calc_action);
}

#endif