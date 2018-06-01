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

/* key open fixed to 0 */
static uint8_t key_open = 0;

/* key-display floor map */
static char keymaps[FLOOR_NUM] = 
{
    -3, -2, -1, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11, 12
};


/**
 * @brief initialize keymap
 * @return init status
 */
bool keymap_init(void)
{
    TRACE("initialize keymap...\r\n");
    uint8_t data[16];
    param_get_keymap(data);
    keymap_update(data);
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
            return i + 1;
        }
    }
    
    return INVALID_KEY;
}

/**
 * @brief convert floor to key
 * @param floor - floor number
 * @return key number, 0xff means error happened
 */
char keymap_key_to_floor(uint8_t key)
{
    key -= 1;
    if (key < FLOOR_NUM)
    {
        return keymaps[key];
    }
    else
    {    
        return INVALID_FLOOR;
    }
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
        keymaps[i] = data[i];
    }
}


