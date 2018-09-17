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
#define HEIGHT_AVERAGE_COUNT       5
#define ALTIMETER_IDLE_INTERVAL    (200 / portTICK_PERIOD_MS)

/**
 * @brief calculate floor height once
 * @param[in ]current_floor: current floor
 * @param[in] distance: distance to top
 */
static uint16_t calc_once(uint8_t current_floor, uint32_t distance)
{
    uint8_t floor_num = board_parameter.total_floor - current_floor;

    if (0 == floor_num)
    {
        TRACE("reach top: %d(mm)\r\n", distance);
        return 0;
    }
    return (uint16_t)(distance / 10.0 / floor_num);
}

/**
 * @brief altimeter receive distance task
 * @param pvParameters - task parameter
 */
static void vAltimeterCalc(void *pvParameters)
{
    uint8_t floor = 1;
    uint16_t heights[HEIGHT_AVERAGE_COUNT];
    memset(heights, 0, HEIGHT_AVERAGE_COUNT * sizeof(uint16_t));
    uint8_t index = 0;
    uint16_t cur_height = 0;
    uint32_t distance = 0;
    for (;;)
    {
        if (CALC_START == calc_action)
        {
            if (xQueueReceive(xQueueCalc, &floor, portMAX_DELAY))
            {
                distance = altimeter_get_distance();
                cur_height = calc_once(floor, distance);
                TRACE("calculated height: %d(cm), %d(mm)\r\n", cur_height, distance);
                if (0 != cur_height)
                {
                    heights[index] = cur_height;
                    index ++;
                    if (HEIGHT_AVERAGE_COUNT == index)
                    {
                        index = 0;
                    }
                }

                uint32_t sum = 0;
                uint32_t valid = 0;
                for (uint8_t i = 0; i < HEIGHT_AVERAGE_COUNT; ++i)
                {
                    if (0 != heights[i])
                    {
                        valid ++;
                        sum += heights[i];
                    }
                }
                if (valid > 0)
                {
                    board_parameter.floor_height = sum / valid;
                    param_store_floor_height(board_parameter.floor_height);
                    TRACE("average height: %d\r\n", board_parameter.floor_height);
                }
                else
                {
                    TRACE("no valid data, use default height: %d\r\n", board_parameter.floor_height);
                }
                notify_calc(board_parameter.floor_height, distance / 10);
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
        TRACE("rebooting...\r\n");
        SCB_SystemReset();
    }
    else
    {
        uint8_t floor = 1;
        while (floor <= board_parameter.total_floor)
        {
            elev_go(floor);
        }
    }
    return TRUE;
}

bool altimeter_is_calculating(void)
{
    return (CALC_START == calc_action);
}

uint16_t alitmeter_floor_height(void)
{
    return board_parameter.floor_height;
}

#endif