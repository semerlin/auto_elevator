/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "floormap.h"
#include "assert.h"
#include "trace.h"
#include "keymap.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[PARAM]"


/**
 * @brief convert display floor to physical floor
 * @param floor - display floor
 * @return physical floor
 */
uint8_t floormap_dis_to_phy(char floor)
{
    return keymap_floor_to_key(floor);
}

/**
 * @brief convert physical floor to display floor
 * @param floor - physical floor
 * @return display floor
 */
char floormap_phy_to_dis(uint8_t floor)
{
    return keymap_key_to_floor(floor);
}