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
#include "global.h"


#undef __TRACE_MODULE
#define __TRACE_MODULE  "[keymap]"

extern parameters_t board_parameter;
/* total floor number */
#ifdef __MASTER
#define FLOOR_NUM    15
#else
#define FLOOR_NUM    16
#endif

#ifdef __MASTER
/* key open fixed to 0 */
static uint8_t key_open = 0;
#endif

/* key-display floor map */
static char keymaps[FLOOR_NUM] =
{
    -3, -2, -1, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11, 12
    };

/**
 * @brief update key map
 * @param data - new key map
 */
static void keymap_update(char floor)
{
    for (int i = 0; i < FLOOR_NUM; ++i)
    {
        if (0 == floor)
        {
            floor ++;
        }
        keymaps[i] = floor;
        floor ++;
    }
}

/**
 * @brief initialize keymap
 * @return init status
 */
bool keymap_init(void)
{
    TRACE("initialize keymap...\r\n");
    char floor = -3;
    floor = board_parameter.start_floor;
    keymap_update(floor);
    return TRUE;
}

/**
 * @brief convert floor to key
 * @param floor - floor number
 * @return key number, 0xff means error happened
 */
uint8_t keymap_floor_to_key(char floor)
{
    for (int i = 0; i < FLOOR_NUM; ++i)
    {
        if (floor == keymaps[i])
        {
#ifdef __MASTER
            return i + 1;
#else
            return i;
#endif
        }
    }

    return INVALID_KEY;
}

/**
 * @brief convert key to floor
 * @param floor - floor number
 * @return key number, 0xff means error happened
 */
char keymap_key_to_floor(uint8_t key)
{
#ifdef __MASTER
    key -= 1;
#endif
    if (key < FLOOR_NUM)
    {
        return keymaps[key];
    }
    else
    {
        return INVALID_FLOOR;
    }
}

#ifdef __MASTER
/**
 * @brief get open evelator key
 * @return open key
 */
uint8_t keymap_open(void)
{
    return key_open;
}
#endif

/**
 * @brief check whether contains specified floor
 * @pram floor - floor to check
 * @return contains or not
 */
bool keymap_floor_contains(char floor)
{
    for (int i = 0; i < FLOOR_NUM; ++i)
    {
        if (floor == keymaps[i])
        {
            return TRUE;
        }
    }

    return FALSE;
}

