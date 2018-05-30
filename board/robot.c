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


/**
 * @brief initialize robot
 * @return init status
 */
bool robot_init(void)
{
    return TRUE;
}


