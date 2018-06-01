/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "elevator.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "led_status.h"
#include "protocol.h"
#include "keymap.h"
#include "keyctl.h"
#include "robot.h"
#include "floormap.h"
#include "global.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[elev]"


/* elevator current floor */
static char floor = 1;

static bool hold_door = FALSE;
static uint8_t hold_cnt = 0;

/* elevator state */
static elev_run_state run_state = run_stop;
static elev_work_state work_state = work_idle;

/**
 * @brief elevator task
 * @param pvParameters - task parameters
 */
static void vElev(void *pvParameters)
{
    for (;;)
    {
        if (hold_door)
        {
            hold_cnt ++;
            if (hold_cnt > 30)
            {
                hold_door = FALSE;
                hold_cnt = 0;
                keyctl_release(keymap_open());
            }
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief initialize elevator
 * @return init status
 */
bool elev_init(void)
{
    xTaskCreate(vElev, "protocol", ELEV_STACK_SIZE, NULL,
                    ELEV_PRIORITY, NULL);
    return TRUE;
}

/**
 * @brief elevator going floor
 * @param floor - going floor
 */
void elev_go(char floor)
{
    uint8_t key = keymap_floor_to_key(floor);
    keyctl_press(key);
}

/**
 * @brief indicate elevator arrive
 * @param floor - arrive floor
 */
void elev_arrived(char floor)
{
    if (work_robot == work_state)
    {
        if (robot_is_checkin(floormap_dis_to_phy(floor)))
        {
            notify_arrive(floor);
        }
    }
}

/**
 * @brief hold elevator door open
 * @param flag - open or close
 */
void elev_hold_open(bool flag)
{
    uint8_t key = keymap_open();
    if (key)
    {
        hold_cnt = 0;
        hold_door = TRUE;
        keyctl_press(key);
    }
    else
    {
        hold_door = FALSE;
        hold_cnt = 0;
        keyctl_release(key);
    }
}

/**
 * @brief decrease current floor
 */
void elev_decrease(void)
{
    floor --;
    if (0 == floor)
    {
        floor --;
    }
    TRACE("decrease floor: %d", floor);
    if (is_down_led_on(floor))
    {
        run_state = run_down;
    }
    else if (is_up_led_on(floor))
    {
        run_state = run_up;
    }
    else
    {
        run_state = run_stop;
    }
}

/**
 * @brief increase current floor
 */
void elev_increase(void)
{
    floor ++;
    if (0 == floor)
    {
        floor ++;
    }
    TRACE("increase floor: %d", floor);
    if (is_up_led_on(floor))
    {
        run_state = run_up;
    }
    else if (is_down_led_on(floor))
    {
        run_state = run_down;
    }
    else
    {
        run_state = run_stop;
    }
}

/**
 * @brief set current floor as first floor
 */
void elev_set_first_floor(void)
{
    floor = 1;
}

/**
 * @brief get elevator previous run status
 * @return floor previous run status
 */
elev_run_state elev_state_run(void)
{
    return run_state;
}

/**
 * @brief get elevator current state
 * @return elevator current state
 */
elev_work_state elev_state_work(void)
{
    return work_state;
}

/**
 * @brief set elevator state
 * @param state - elevator state
 */
void elevator_set_state_work(elev_work_state state)
{
    work_state = state;
}

/**
 * @brief get elevator current floor
 * @return elevator current floor
 */
char elev_floor(void)
{
    return floor;
}



