/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _BOARDMAP_H_
#define _BOARDMAP_H_

#include "types.h"
#include "config.h"

BEGIN_DECLS

typedef struct
{
    /** 0 means empty */
    uint8_t id_board;
    uint8_t start_key;
    uint8_t start_floor;
    uint8_t floor_num;
    uint16_t led_status;
} boardmap_t;
extern boardmap_t boardmaps[MAX_BOARD_NUM];

bool boardmap_add(uint8_t id_board, uint8_t start_key, char start_floor,
                  uint8_t floor_num, uint16_t led_status);
uint8_t boardmap_floor_to_key(uint8_t floor);
uint8_t boardmap_key_to_floor(uint8_t id_board, uint8_t key);
uint8_t boardmap_get_floor_board_id(uint8_t floor);
void boardmap_update_led_status(uint8_t id_board, uint16_t led_status);
uint16_t boardmap_get_led_status(uint8_t id_board);
#ifdef __MASTER
bool boardmap_is_board_id_exists(uint8_t id_board);
uint8_t boardmap_opendoor_key(void);
#endif

END_DECLS

#endif
