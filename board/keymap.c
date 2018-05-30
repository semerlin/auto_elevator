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
    char floor;
    uint8_t key;
}keymap;

/* key-floor map */
static keymap keymaps[FLOOR_NUM] = 
{
    {-3, 0},
    {-2, 1},
    {-1, 2},
    {1, 3},
    {2, 4},
    {3, 5},
    {4, 6},
    {5, 7},
    {6, 8},
    {7, 9},
    {8, 10},
    {9, 11},
    {10, 12},
    {11, 13},
    {12, 14},
};

static uint8_t key_open = 15;

/**
 * @brief initialize keymap
 * @return init status
 */
bool keymap_init(void)
{
    TRACE("initialize keymap...\r\n");
    uint8_t data[36];
    if (is_param_setted())
    {
        param_get_keymap(data);
        keymap_update((const char *)data);
    }
    return TRUE;
}

/**
 * @brief convert floor to key
 * @param floor - floor number
 * @return key number, 0xff means error happened
 */
uint8_t keymap_floor_to_key(char floor);
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
char keymap_key_to_floor(uint8_t key);
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
void keymap_update(const char *data)
{
    for (int i = 0; i < FLOOR_NUM; ++i)
    {
        keymaps[i].floor = data[i * 2];
        keymaps[i].key = data[i * 2 + 1];
    }
    key_open = (uint8_t)(data[FLOOR_NUM * 2]);
}


