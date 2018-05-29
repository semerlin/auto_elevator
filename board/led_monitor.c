/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "led_monitor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[LED]"

bool led_monitor_init(void)
{
    return TRUE;
}