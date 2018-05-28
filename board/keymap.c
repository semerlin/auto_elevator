/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "keymap.h"

#define FLOOR_NUM    18

typedef struct
{
    int floor;
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
    {13, 15},
    {14, 16},
    {15, 17},
};


/**
 * @brief initialize keymap
 * @return init status
 */
bool keymap_init(void)
{
    return TRUE;
}

/**
 * @brief convert floor to key
 * @param floor - floor number
 * @return key number, 0xff means error happened
 */
uint8_t keymap_convert(int floor)
{
    for (int i = 0; i < FLOOR_NUM; ++i)
    {
        if (floor == keymaps[i].floor)
        {
            return keymaps[i].key;
        }
    }
    
    return 0xff;
}



