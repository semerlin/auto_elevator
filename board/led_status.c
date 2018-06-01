/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "led_status.h"
#include "assert.h"
#include "trace.h"
#include "cm3_core.h"
#include "pinconfig.h"
#include "keymap.h"

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
 * @brief clear open led
 * @param status - led status
 */
static __INLINE void clear_open_led(uint16_t status)
{
    status &= ~0x01;
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
uint16_t led_status_get(void)
{
    return hc166_readdata();
}

/**
 * @brief check floor led status
 * @param floor - specified floor
 * @return floor led on/off status
 */
bool is_led_on(char floor)
{
    if (floor > LED_NUM)
    {
        return FALSE;
    }
    uint16_t status = hc166_readdata();
    clear_open_led(status);
    uint8_t key = keymap_floor_to_key(floor);
    if (INVALID_KEY != key)
    {
        return (0 != (status & (1 << key)));
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief check down floor led status
 * @param floor - specified floor
 * @return floor led on/off status
 */
bool is_down_ledon(char floor)
{
    if (floor > LED_NUM)
    {
        return FALSE;
    }
    uint16_t status = hc166_readdata();
    clear_open_led(status);
    uint8_t key = keymap_floor_to_key(floor);
    if (INVALID_KEY != key)
    {
        uint16_t floor_bit = (1 << key);
        floor_bit -= 1;
        return (0 != (status & floor_bit));
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief check up floor led status
 * @param floor - specified floor
 * @return floor led on/off status
 */
bool is_up_ledon(char floor)
{
    if (floor > LED_NUM)
    {
        return FALSE;
    }
    uint16_t status = hc166_readdata();
    clear_open_led(status);
    uint8_t key = keymap_floor_to_key(floor);
    if (INVALID_KEY != key)
    {
        uint16_t floor_bit = (1 << key);
        floor_bit |= (floor_bit - 1);
        return (0 != (status & ~floor_bit));
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief check floor led status except floor specified
 * @param floor - specified floor
 * @return led status
 */
bool is_ledon_except(char floor)
{
    if (floor > LED_NUM)
    {
        return FALSE;
    }
    uint16_t status = hc166_readdata();
    clear_open_led(status);
    uint8_t key = keymap_floor_to_key(floor);
    if (INVALID_KEY != key)
    {
        uint16_t floor_bit = (1 << key);
        return (0 != (status & ~floor_bit));
    }
    else
    {
        return FALSE;
    }
}
