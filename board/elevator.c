/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "elevator.h"
#include "trace.h"
#include "ledstatus.h"
#include "protocol.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[elev]"


/* elevator current floor */
static uint8_t floor = 1;

/* elevator state */
static elev_run_state run_state = run_stop;
static elev_work_state work_state = work_idle;

/**
 * @brief initialize elevator
 * @return init status
 */
bool elev_init(void)
{
    return TRUE;
}

/**
 * @brief elevator going floor
 * @param floor - going floor
 */
static void elev_go(uint8_t floor)
{
}

/**
 * @brief indicate elevator arrive
 * @param floor - arrive floor
 */
void elev_arrived(uint8_t floor)
{
    if (work_robot == work_state)
    {
        /* 增加是否登记楼层判断，只申请了电梯可以不发送 */
        notify_arrive(floor);
    }
}

/**
 * @brief hold elevator door open
 * @param flag - open or close
 */
void elev_hold_open(bool flag)
{
}

/**
 * @brief decrease current floor
 */
void elev_decrease(void)
{
    floor --;
    TRACE("decrease floor: %d", elev_floor);
    if (is_down_ledon(floor))
    {
        run_state = run_down;
    }
    else if (is_up_ledon(floor))
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
    TRACE("increase floor: %d", elev_floor);
    if (is_up_ledon(floor))
    {
        run_state = run_up;
    }
    else if (is_down_ledon(floor))
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
elev_work_state elevator_state_work(void)
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
uint8_t elev_floor(void)
{
    return floor;
}



