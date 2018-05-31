/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "keymap.h"
#include "assert.h"
#include "trace.h"
#include "parameter.h"


#undef __TRACE_MODULE
#define __TRACE_MODULE  "[KEYMAP]"

/* total floor number */
#define FLOOR_NUM    15

/* keymap structure */
typedef struct
{
    uint8_t floor;
    uint8_t key;
}keymap;

/* key-floor map */
static keymap keymaps[FLOOR_NUM] = 
{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
    {7, 7},
    {8, 8},
    {9, 9},
    {10, 10},
    {11, 11},
    {12, 12},
    {13, 13},
    {14, 14},
    {15, 15},
};

static uint8_t key_open = 0;

/**
 * @brief initialize keymap
 * @return init status
 */
bool keymap_init(void)
{
    TRACE("initialize keymap...\r\n");
#if 0
    uint8_t data[36];
    if (is_param_setted())
    {
        param_get_keymap(data);
        keymap_update(data);
    }
#endif
    return TRUE;
}

/**
 * @brief convert floor to key
 * @param floor - floor number
 * @return key number, 0xff means error happened
 */
uint8_t keymap_floor_to_key(uint8_t floor)
{
    for (int i = 0; i < FLOOR_NUM; ++i)
    {
        if (floor == keymaps[i].floor)
        {
            return keymaps[i].key;
        }
    }
    
    return INVALID_KEY;
}

/**
 * @brief convert floor to key
 * @param floor - floor number
 * @return key number, 0xff means error happened
 */
uint8_t keymap_key_to_floor(uint8_t key)
{
    for (int i = 0; i < FLOOR_NUM; ++i)
    {
        if (key == keymaps[i].key)
        {
            return keymaps[i].floor;
        }
    }
    
    return INVALID_FLOOR;
}

/**
 * @brief get open evelator key
 * @return open key
 */
uint8_t keymap_open(void)
{
    return key_open;
}

/**
 * @brief update key map
 * @param data - new key map
 */
void keymap_update(const uint8_t *data)
{
    for (int i = 0; i < FLOOR_NUM; ++i)
    {
        keymaps[i].floor = data[i * 2];
        keymaps[i].key = data[i * 2 + 1];
    }
    key_open = data[FLOOR_NUM * 2];
}


