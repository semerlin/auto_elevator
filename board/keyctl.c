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
#define __TRACE_MODULE  "[keyctl]"

#define KEY_NUM 16 

/* led status */
static uint16_t key_status = 0xffff;


/**
 * @brief shift register transition
 */
static __INLINE void sh_transition(void)
{
    pin_reset("KEY_SH");
    __NOP();
    __NOP();
    pin_set("KEY_SH");
    __NOP();
    __NOP();
}

/**
 * @brief storage registers transition
 */
static __INLINE void st_transition(void)
{
    pin_reset("KEY_ST");
    __NOP();
    __NOP();
    pin_set("KEY_ST");
    __NOP();
    __NOP();
}

/**
 * @brief send data to 74hc595
 * @param data - data to send
 */
static void hc595_senddata(uint16_t data)
{
    for (int i = 0; i < 16; ++i)
    {
        if (0 != (data & 0x8000))
        {
            pin_reset("KEY_DATA");
        }
        else
        {
            pin_set("KEY_DATA");
        }
        data <<= 1;
        sh_transition();
    }
    st_transition();
}

/**
 * @brief initialize key control 
 */
void keyctl_init(void)
{
    TRACE("initialieze key control...\r\n");
    key_status = 0xffff;
    hc595_senddata(key_status);
}

/**
 * @brief press key
 * @param num - key number(0-15)
 */
void keyctl_press(uint8_t num)
{
    assert_param(num < KEY_NUM);
    TRACE("press key: %d\r\n", num);
    key_status &= ~(1 << num);
    hc595_senddata(key_status);
}

/**
 * @brief release key
 * @param num - key number(0-15)
 */
void keyctl_release(uint8_t num)
{
    assert_param(num < KEY_NUM);
    TRACE("release key: %d\r\n", num);
    key_status |= (1 << num);
    hc595_senddata(key_status);
}

/**
 * @brief release all keys
 */
void keyctl_release_all(void)
{
    TRACE("release all keys\r\n");
    key_status = 0xffff;
    hc595_senddata(key_status);
}

