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

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[robot]"

typedef struct
{
    bool used;
    uint8_t id;
    uint8_t floor;
}robot_checkin;

robot_checkin checkins[5];

#define DEFAULT_CHECKIN   0xf7


/**
 * @brief initialize robot
 */
void robot_init(void)
{
    for (int i = 0; i < sizeof(checkins) / sizeof(checkins[0]); ++i)
    {
        checkins[i].used = FALSE;
        checkins[i].id = 0;
        checkins[i].floor = 0;
    }
}

/**
 * @brief set robot checkin floor
 * @param floor - check in floor
 */
bool robot_checkin_set(uint8_t id, uint8_t floor)
{
    for (int i = 0; i < sizeof(checkins) / sizeof(checkins[0]); ++i)
    {
        if (!checkins[i].used)
        {
            checkins[i].used = TRUE;
            checkins[i].id = id;
            checkins[i].floor = floor;
            return TRUE;
        }
    }
    
    return FALSE;
}

/**
 * @brief reset robot checkin floor
 */
void robot_checkin_reset(uint8_t id)
{
    for (int i = 0; i < sizeof(checkins) / sizeof(checkins[0]); ++i)
    {
        if (checkins[i].used && (id == checkins[i].id))
        {
            checkins[i].used = FALSE;
            checkins[i].id = id;
            checkins[i].floor = DEFAULT_CHECKIN;
            break;
        }
    }    
}

/**
 * @brief get robot checkin floor
 * @return checkin floor
 */
uint8_t robot_checkin_get(uint8_t id)
{
    for (int i = 0; i < sizeof(checkins) / sizeof(checkins[0]); ++i)
    {
        if (checkins[i].used && (id == checkins[i].id))
        {
            return checkins[i].floor;
        }
    }
    
    return DEFAULT_CHECKIN;
}

/**
 * @brief get robot id
 * @return robot id
 */
uint8_t robot_id_get(uint8_t floor)
{
    for (int i = 0; i < sizeof(checkins) / sizeof(checkins[0]); ++i)
    {
        if (checkins[i].used && (floor == checkins[i].floor))
        {
            return checkins[i].id;
        }
    }
    
    return 0xff;
}


