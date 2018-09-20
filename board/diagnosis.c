/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "diagnosis.h"
#include "trace.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[diagnosis]"

bool diagnosis_init(void)
{
    TRACE("initialize diagnosis system...\r\n");
    return TRUE;
}

void dump_register(void)
{
}

void HardFaultException(void)
{
    TRACE("hardfault happened!\r\n");
    dump_register();
}

void BusFaultException(void)
{
    TRACE("busfault happened!\r\n");
    dump_register();
}
