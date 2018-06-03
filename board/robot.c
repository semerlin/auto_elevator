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
    uint8_t id;
    uint8_t floor;
}robot_checkin;

robot_checkin checkin;




/**
 * @brief initialize robot
 */
void robot_init(void)
{
    TRACE("initialize robot...\r\n");
    checkin.id = 0;
    checkin.floor = DEFAULT_CHECKIN;
}

/**
 * @brief set robot checkin floor
 * @param floor - check in floor
 */
bool robot_checkin_set(uint8_t id, uint8_t floor)
{
    checkin.id = id;
    checkin.floor = floor;
    return TRUE;
}

/**
 * @brief reset robot checkin floor
 */
void robot_checkin_reset(uint8_t id)
{
    checkin.id = 0;
    checkin.floor = DEFAULT_CHECKIN;   
}

/**
 * @brief get robot checkin floor
 * @return checkin floor
 */
uint8_t robot_checkin_get(uint8_t id)
{
    if (id == checkin.id)
    {
        return checkin.floor;
    }
    
    return DEFAULT_CHECKIN;
}

/**
 * @brief get current robot checkin floor
 * @return current robot checkin floor
 */
uint8_t robot_checkin_cur()
{
    return checkin.floor;
}

/**
 * @brief get robot id
 * @return robot id
 */
uint8_t robot_id_get(uint8_t floor)
{
    if (floor == checkin.floor)
    {
        return checkin.id;
    }
    
    return 0xff;
}

/**
 * @brief check if specified floor is checked in
 * @param floor - specified floor
 * @return check status
 */
bool robot_is_checkin(uint8_t floor)
{
    return (floor == checkin.floor);
}


