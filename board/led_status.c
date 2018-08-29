/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include <string.h>
#include "led_status.h"
#include "assert.h"
#include "trace.h"
#include "cm3_core.h"
#include "pinconfig.h"
#include "boardmap.h"
#include "expand.h"
#include "parameter.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ledstatus]"

/** 0 means led on, 1 means led off */

extern parameters_t board_parameter;
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

#ifdef __MASTER
/**
 * @brief clear open led
 * @param status - led status
 */
#define CLEAR_OPEN_LED(status) (status |= 0x01)
#endif

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
    /* read left data */
    st_set();
    for (int i = 0; i < LED_NUM - 1; ++i)
    {
        sh_transition();
        recvdata <<= 1;
        if (is_pinset("LED_DATA"))
        {
            recvdata |= 0x01;
        }
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

#ifdef __MASTER
/**
 * @brief check floor led status
 * @param floor - specified floor
 * @return floor led on/off status
 */
bool is_led_on(char floor)
{
    uint8_t key = INVALID_KEY;
    uint16_t status = 0;

    /** get floor board id */
    uint8_t id_board = boardmap_get_floor_board_id(floor);
    if (ID_BOARD_INVALID == id_board)
    {
        return FALSE;
    }

    /** get board led status */
    status = boardmap_get_led_status(id_board);

    /** get floor key */
    key = boardmap_floor_to_key(floor);
    if (INVALID_KEY == key)
    {
        return FALSE;
    }

    return (0 == (status & (1 << key)));
}

/**
 * @brief get floor in boardmap index
 * @param[in] floor: floor to process
 * @return floot in boardmap index
 */
static uint8_t get_floor_map_index(char floor)
{
    char start_floor, end_floor;
    uint8_t i = 0;
    for (; i < MAX_BOARD_NUM; ++i)
    {
        if (0 != boardmaps[i].start_floor)
        {
            start_floor = boardmaps[i].start_floor;
            end_floor = boardmaps[i].start_floor + boardmaps[i].floor_num;
            if ((start_floor < 0) && (end_floor >= 0))
            {
                end_floor ++;
            }

            if ((floor >= start_floor) && (floor <= end_floor))
            {
                break;
            }
        }
    }

    return i;
}
/**
 * @brief check down floor led status
 * @param floor - specified floor
 * @return floor led on/off status
 */
bool is_down_led_on(char floor)
{
    uint8_t index = get_floor_map_index(floor);
    if (index >= MAX_BOARD_NUM)
    {
        return FALSE;
    }

    uint16_t led_status[MAX_BOARD_NUM];
    uint16_t *pdata = led_status;
    for (uint8_t i = 0; i <= index; ++i)
    {
        if (0 != boardmaps[i].start_floor)
        {
            *pdata = boardmaps[i].led_status;
            if (ID_BOARD_MASTER == boardmaps[i].id_board)
            {
                CLEAR_OPEN_LED(*pdata);
            }
            pdata++;
        }
    }

    /** check lower index led status */
    uint8_t valid_len = (pdata - led_status) / sizeof(uint16_t);
    for (int8_t i = 0; i < valid_len - 1; ++i)
    {
        if (led_status[i] < 0xffff)
        {
            return TRUE;
        }
    }

    /** check board in led status */
    uint8_t key = boardmap_floor_to_key(floor);
    uint16_t cur_status = led_status[valid_len - 1];
    for (uint8_t i = 0; i < key; ++i)
    {
        if (0 == (cur_status & (1 << key)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/**
 * @brief check up floor led status
 * @param floor - specified floor
 * @return floor led on/off status
 */
bool is_up_led_on(char floor)
{
    uint8_t index = get_floor_map_index(floor);
    if (index >= MAX_BOARD_NUM)
    {
        return FALSE;
    }

    uint16_t led_status[MAX_BOARD_NUM];
    uint16_t *pdata = led_status;
    for (uint8_t i = index; i < MAX_BOARD_NUM; ++i)
    {
        if (0 != boardmaps[i].start_floor)
        {
            *pdata = boardmaps[i].led_status;
            if (ID_BOARD_MASTER == boardmaps[i].id_board)
            {
                CLEAR_OPEN_LED(*pdata);
            }
            pdata++;
        }
    }

    /** check upper index led status */
    uint8_t valid_len = (pdata - led_status) / sizeof(uint16_t);
    for (int8_t i = 1; i < valid_len; ++i)
    {
        if (led_status[i] < 0xffff)
        {
            return TRUE;
        }
    }

    /** check board in led status */
    uint8_t key = boardmap_floor_to_key(floor);
    uint16_t cur_status = led_status[0];
    for (uint8_t i = key + 1; i < boardmaps[index].floor_num; ++i)
    {
        if (0 == (cur_status & (1 << key)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

#endif

