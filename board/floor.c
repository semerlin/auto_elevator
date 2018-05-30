/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include <string.h>
#include "floor.h"
#include "assert.h"
#include "trace.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[floor]"

#define FLOOR_ARRIVE  0
#define FLOOR_UP      1
#define FLOOR_DOWN    2

/* elevator current floor */
static char floor = 1;

/* floor state */
static uint8_t floor_state_prev = FLOOR_ARRIVE;

bool floor_init(void)
{
}

static void floor_enter(char floor)
{
}


void floor_arrived(char floor)
{
}

void floor_hold_open(char floor)
{
}

/**
 * @brief get floor previous status
 * @return floor previous status
 */
uint8_t floor_prev_state(void)
{
    return floor_state_prev;
}

/**
 * @brief decrease current floor
 */
void floor_decrease(void)
{
    floor --;
    if (0 == floor)
    {
        floor --;
    }
    TRACE("decrease floor: %d", floor);
    floor_state_prev = ELEV_DOWN;
}

/**
 * @brief increase current floor
 */
void floor_increase(void);
{
    floor ++;
    if (0 == floor)
    {
        floor ++;
    }
    TRACE("increase floor: %d", floor);
    floor_state_prev = ELEV_UP;
}

/**
 * @brief set current floor as first floor
 */
void floor_set_first(void)
{
    floor = 1;
}


