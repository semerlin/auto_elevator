/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "boardmap.h"
#include "assert.h"
#include "trace.h"
#include "parameter.h"
#include "global.h"
#include "expand.h"
#include "dbgserial.h"
#include "config.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[board_info]"

extern parameters_t board_parameter;
boardmap_t boardmaps[MAX_BOARD_NUM];


#ifdef __MASTER
/**
 * @brief get open evelator key
 * @return open key
 */
uint8_t boardmap_opendoor_key(void)
{
    /* open door key fixed to 0 */
    return 0;
}

/**
 * @brief check whether specified board exists
 * @param[in] id_board: board id need to be checked
 * @retval TRUE: specified board exist
 * @retval FALSE: specified board does not exist
 */
bool boardmap_is_board_id_exists(uint8_t id_board)
{
    for (uint8_t i = 0; i < MAX_BOARD_NUM; ++i)
    {
        if (id_board == boardmaps[i].id_board)
        {
            return TRUE;
        }
    }

    return FALSE;
}
#endif

/**
 * @brief sort boardmap
 */
static void boardmap_sort(void)
{
    for (uint8_t i = 0; i < MAX_BOARD_NUM; ++i)
    {
        for (uint8_t j = 0; j < MAX_BOARD_NUM - i - 1; ++j)
        {
            if (boardmaps[j].start_floor > boardmaps[j + 1].start_floor)
            {
                boardmap_t temp_map = boardmaps[j];
                boardmaps[j] = boardmaps[j + 1];
                boardmaps[j + 1] = temp_map;
            }
        }
    }
}

#if DUMP_BOARDMAP
/**
 * @brief dump message
 * @param data - message to dump
 * @param len - data length
 */
static void dump_message(void)
{
    TRACE("boardmap: \r\n");

    for (uint8_t i = 0; i < MAX_BOARD_NUM; ++i)
    {
        if (0 != boardmaps[i].id_board)
        {
            dbg_putstring("id: ", 4);
            dbg_putchar("0123456789abcdef"[boardmaps[i].id_board >> 4]);
            dbg_putchar("0123456789abcdef"[boardmaps[i].id_board & 0x0f]);
            dbg_putchar(' ');

            dbg_putstring("start key: ", 11);
            dbg_putchar("0123456789abcdef"[boardmaps[i].start_key >> 4]);
            dbg_putchar("0123456789abcdef"[boardmaps[i].start_key & 0x0f]);
            dbg_putchar(' ');

            dbg_putstring("start floor: ", 13);
            dbg_putchar("0123456789abcdef"[(uint8_t)(boardmaps[i].start_floor) >> 4]);
            dbg_putchar("0123456789abcdef"[(uint8_t)(boardmaps[i].start_floor) & 0x0f]);
            dbg_putchar(' ');

            dbg_putstring("floor num: ", 11);
            dbg_putchar("0123456789abcdef"[boardmaps[i].floor_num >> 4]);
            dbg_putchar("0123456789abcdef"[boardmaps[i].floor_num & 0x0f]);

            dbg_putstring("\r\n", 2);
        }
    }
}
#endif

/**
 * @param add new board to board map
 */
bool boardmap_add(uint8_t id_board, uint8_t start_key, char start_floor,
                  uint8_t floor_num, uint16_t led_status)
{
    for (uint8_t i = 0; i < MAX_BOARD_NUM; ++i)
    {
        if (0 == boardmaps[i].id_board)
        {
            boardmaps[i].id_board = id_board;
            boardmaps[i].start_key = start_key;
            if (0 == start_floor)
            {
                boardmaps[i].start_floor = 1;
            }
            else
            {
                boardmaps[i].start_floor = start_floor;
            }
            boardmaps[i].floor_num = floor_num;
            boardmaps[i].led_status = led_status;
            boardmap_sort();
#if DUMP_BOARDMAP
            dump_message();
#endif
            return TRUE;
        }
    }

    return FALSE;
}

/**
 * @brief convert floor to key
 * @param floor - floor number
 * @return key number, 0xff means error happened
 */
uint8_t boardmap_floor_to_key(char floor)
{
    if (0 == floor)
    {
        return INVALID_KEY;
    }

    char start_floor, end_floor;
    uint8_t key = INVALID_KEY;
    for (uint8_t i = 0; i < MAX_BOARD_NUM; ++i)
    {
        start_floor = boardmaps[i].start_floor;
        end_floor = boardmaps[i].start_floor + boardmaps[i].floor_num;
        if ((start_floor < 0) && (end_floor >= 0))
        {
            end_floor ++;
        }
        if ((floor >= start_floor) && (floor <= end_floor))
        {
            if ((floor > 0) && (start_floor < 0))
            {
                key = floor - start_floor - 1;
            }
            else
            {
                key = floor - start_floor;
            }
            key += boardmaps[i].start_key;
            break;
        }
    }

    return key;
}

/**
 * @brief convert key to floor
 * @param floor - floor number
 * @return floor number, 0x7f means error happened
 */
char boardmap_key_to_floor(uint8_t id_board, uint8_t key)
{
    char floor = INVALID_FLOOR;
    for (int i = 0; i < MAX_BOARD_NUM; ++i)
    {
        if (id_board == boardmaps[i].id_board)
        {
            if ((key < boardmaps[i].start_key + boardmaps[i].floor_num) &&
                (key >= boardmaps[i].start_key))
            {
                floor = boardmaps[i].start_floor + key - boardmaps[i].start_key;
            }
            break;
        }
    }

    return floor;
}

/**
 * @brief get specified floor board id
 * @param floor - floor number
 * @return board id, 0xff means error happened
 */
uint8_t boardmap_get_floor_board_id(char floor)
{
    if (0 == floor)
    {
        return ID_BOARD_INVALID;
    }

    uint8_t id_board = ID_BOARD_INVALID;
    char start_floor, end_floor;
    for (uint8_t i = 0; i < MAX_BOARD_NUM; ++i)
    {
        start_floor = boardmaps[i].start_floor;
        end_floor = boardmaps[i].start_floor + boardmaps[i].floor_num;
        if ((start_floor < 0) && (end_floor >= 0))
        {
            end_floor ++;
        }
        if ((floor >= start_floor) && (floor <= end_floor))
        {
            id_board = boardmaps[i].id_board;
            break;
        }
    }

    return id_board;
}

/**
 * @brief update board led status
 * @param[in] id_board: board id
 * @param[in] led_status: new les status
 */
void boardmap_update_led_status(uint8_t id_board, uint16_t led_status)
{
    for (uint8_t i = 0; i < MAX_BOARD_NUM; ++i)
    {
        if (id_board == boardmaps[i].id_board)
        {
            boardmaps[i].led_status = led_status;
        }
    }
}

/**
 * @brief get specified board led status
 * @param[in] id_board: board id
 * @return board led status
 */
uint16_t boardmap_get_led_status(uint8_t id_board)
{
    for (uint8_t i = 0; i < MAX_BOARD_NUM; ++i)
    {
        if (id_board == boardmaps[i].id_board)
        {
            return boardmaps[i].led_status;
        }
    }

    return 0xffff;
}

