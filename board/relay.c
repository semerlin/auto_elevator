/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "keyctl.h"
#include "assert.h"
#include "trace.h"
#include "cm3_core.h"
#include "pinconfig.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[relay]"

#define RELAY_NUM   2

/**
 * @brief open specified relay
 * @param num - relay number
 */
void relay_open(uint8_t num)
{
    assert_param(num < RELAY_NUM);
    TRACE("open relay: %d\r\n", num);
    if (0 == num)
    {
        pin_set("RELAY1");
    }
    else
    {
        pin_set("RELAY2");
    }
}

/**
 * @brief close specified relay
 * @param num - relay number
 */
void relay_close(uint8_t num)
{
    assert_param(num < RELAY_NUM);
    TRACE("close relay: %d\r\n", num);
    if (0 == num)
    {
        pin_reset("RELAY1");
    }
    else
    {
        pin_reset("RELAY2");
    }
}