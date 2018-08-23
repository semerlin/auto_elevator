/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "robot.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "global.h"
#include "elevator.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[robot]"

typedef struct
{
    uint8_t id;
    uint8_t floor;
} robot_info;

#define DEFAULT_ID    0xff

static robot_info robot = {DEFAULT_ID, DEFAULT_CHECKIN};

/* monitor flag */
static bool robot_monitor = FALSE;

static uint32_t monitot_count = 0;
#define RESET_TIME   (600)

/**
 * @brief led monitor task
 * @param pvParameters - task parameter
 */
static void vRobotMonitor(void *pvParameters)
{
    for (;;)
    {
        if (robot_monitor)
        {
            monitot_count ++;
            if (monitot_count > RESET_TIME)
            {
                monitot_count = 0;
                robot_monitor = FALSE;
                elev_hold_open(FALSE);
                robot_id_reset();
                elevator_set_state_work(work_idle);
            }
        }
        else
        {
            monitot_count = 0;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief initialize robot
 */
void robot_init(void)
{
    TRACE("initialize robot...\r\n");
    robot.id = DEFAULT_ID;
    robot.floor = DEFAULT_CHECKIN;
    xTaskCreate(vRobotMonitor, "robot", ROBOT_STACK_SIZE, NULL,
                ROBOT_PRIORITY, NULL);
}

/**
 * @brief set robot id
 * @param id - robot id
 */
void robot_id_set(uint8_t id)
{
    robot.id = id;
}

/**
 * @brief get robot id
 * @return robot id
 */
uint8_t robot_id_get(void)
{
    return robot.id;
}

/**
 * @brief reset robot id
 */
void robot_id_reset(void)
{
    robot.id = DEFAULT_ID;
    robot.floor = DEFAULT_CHECKIN;
}

/**
 * @brief set robot checkin floor
 * @param floor - check in floor
 */
void robot_checkin_set(uint8_t floor)
{
    robot.floor = floor;
}

/**
 * @brief reset robot checkin floor
 */
void robot_checkin_reset(void)
{
    robot.floor = DEFAULT_CHECKIN;
}

/**
 * @brief get robot checkin floor
 * @return checkin floor
 */
uint8_t robot_checkin_get(void)
{
    return robot.floor;
}

/**
 * @brief check if specified floor is checked in
 * @param floor - specified floor
 * @return check status
 */
bool robot_is_checkin(uint8_t floor)
{
    return (floor == robot.floor);
}

/**
 * @brief start monitor robot status
 */
void robot_monitor_start(void)
{
    robot_monitor = TRUE;
}

/**
 * @brief stop robot monitor status
 */
void robot_monitor_stop(void)
{
    robot_monitor = FALSE;
}

/**
 * @brief reset robot monitor count
 */
void robot_monitor_reset(void)
{
    monitot_count = 0;
}

