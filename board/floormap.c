/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifdef __MASTER
#include <string.h>
#include "floormap.h"
#include "assert.h"
#include "trace.h"
#include "expand.h"
#include "parameter.h"
#include "boardmap.h"
#include "dbgserial.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[FLOORMAP]"

#define DUMP_MESSAGE  1

extern parameters_t board_parameter;
static char floormap[MAX_FLOOR_NUM + MAX_EXPAND_FLOOR_NUM * (MAX_BOARD_NUM - 1)];

#if DUMP_MESSAGE
/**
 * @brief dump message
 * @param data - message to dump
 * @param len - data length
 */
static void dump_message(const uint8_t *data, uint8_t len)
{
    TRACE("floormap: ");

    for (int i = 0; i < len; ++i)
    {
        dbg_putchar("0123456789abcdef"[data[i] >> 4]);
        dbg_putchar("0123456789abcdef"[data[i] & 0x0f]);
        dbg_putchar(' ');
    }
    dbg_putchar('\r');
    dbg_putchar('\n');
}
#endif

/**
 * @brief update phy-dis floormap
 */
void floormap_update(void)
{
    TRACE("updating floormap...\r\n");

    /** fill floormap */
    char *pfloor = floormap;
    char delta = 0;
    for (uint8_t i = 0; i < MAX_BOARD_NUM; ++i)
    {
        delta = 0;
        if (0 != boardmaps[i].start_floor)
        {
            for (uint8_t j = 0; j < boardmaps[i].floor_num; ++j)
            {
                *pfloor = boardmaps[i].start_floor + j + delta;
                if (0 == *pfloor)
                {
                    delta = 1;
                    *pfloor += 1;
                }
                pfloor ++;
            }
        }
    }

#if DUMP_MESSAGE
    dump_message((uint8_t *)floormap, MAX_FLOOR_NUM + MAX_EXPAND_FLOOR_NUM * (MAX_BOARD_NUM - 1));
#endif
}

/**
 * @brief convert display floor to physical floor
 * @param floor - display floor
 * @return physical floor
 */
uint8_t floormap_dis_to_phy(char floor)
{
    for (uint16_t i = 0; i < MAX_FLOOR_NUM +
         MAX_EXPAND_FLOOR_NUM * (MAX_BOARD_NUM - 1); ++i)
    {
        if (floor == floormap[i])
        {
            return i + 1;
        }
    }
    return 0;
}

/**
 * @brief convert physical floor to display floor
 * @param floor - physical floor
 * @return display floor
 */
char floormap_phy_to_dis(uint8_t floor)
{
    return floormap[floor - 1];
}

/**
 * @brief check whether contains specified floor
 * @param[in] floor: floor to check
 *@return check status
 */
bool floormap_contains_floor(char floor)
{
    for (uint16_t i = 0; i < MAX_FLOOR_NUM +
         MAX_EXPAND_FLOOR_NUM * (MAX_BOARD_NUM - 1); ++i)
    {
        if (floor == floormap[i])
        {
            return i + 1;
        }
    }
    return 0;
}
#endif