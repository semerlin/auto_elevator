/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "hp5803.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[HP5803]"

bool hp5803_init(void)
{
    return TRUE;
}