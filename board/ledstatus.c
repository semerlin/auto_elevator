/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "ledstatus.h"
#include "assert.h"
#include "trace.h"
#include "cm3_core.h"
#include "pinconfig.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ledstatus]"

#define LED_NUM   16

/**
 * @brief shift register transition
 */
static __INLINE void sh_transition(void)
{
    pin_reset("LED_SH");
    __NOP();
    __NOP();
    pin_set("LED_SH");
    __NOP();
    __NOP();
}

/**
 * @brief storage registers set 
 */
static __INLINE void st_set(void)
{
    pin_set("LED_ST");
    __NOP();
    __NOP();
}

/**
 * @brief storage registers reset 
 */
static __INLINE void st_reset(void)
{
    pin_reset("LED_ST");
    __NOP();
    __NOP();
}

/**
 * @brief send data to 74hc595
 * @param data - data to send
 */
static uint16_t hc166_readdata(void)
{
    uint16_t recvdata = 0;
    /* parallel load */
    st_reset();
    sh_transition();
    /* read highest data */
    if (is_pinset("LED_DATA"))
    {
        recvdata |= 0x01;
    }
    recvdata <<= 1;
    /* read left data */
    st_set();
    for (int i = 0; i < LED_NUM - 1; ++i)
    {
        sh_transition();
        if (is_pinset("LED_DATA"))
        {
            recvdata |= 0x01;
        }
        recvdata <<= 1;
    }
    
    return recvdata;
}

/**
 * @brief get led status
 * @return led status
 */
uint16_t ledstatus_get(void)
{
    return hc166_readdata();
}

