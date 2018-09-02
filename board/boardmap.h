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

BEGIN_DECLS

#define INVALID_FLOOR    125
#define INVALID_KEY      0xff

#define ID_BOARD_MASTER          0x01
#define ID_BOARD_INVALID         0xff

#ifdef __MASTER
#define MAX_FLOOR_NUM           15
#define MAX_EXPAND_FLOOR_NUM    16
#define MAX_BOARD_NUM           6
#define START_KEY               1
#define EXPAND_START_KEY        0
#else
#define MAX_BOARD_NUM           1
#define MAX_FLOOR_NUM           16
#define START_KEY               0
#endif

typedef struct
{
    /** 0 means empty */
    uint8_t id_board;
    uint8_t start_key;
    char start_floor;
    uint8_t floor_num;
    uint16_t led_status;
} boardmap_t;
extern boardmap_t boardmaps[MAX_BOARD_NUM];

bool boardmap_add(uint8_t id_board, uint8_t start_key, char start_floor,
                  uint8_t floor_num, uint16_t led_status);
uint8_t boardmap_floor_to_key(char floor);
char boardmap_key_to_floor(uint8_t id_board, uint8_t key);
uint8_t boardmap_get_floor_board_id(char floor);
void boardmap_update_led_status(uint8_t id_board, uint16_t led_status);
uint16_t boardmap_get_led_status(uint8_t id_board);
#ifdef __MASTER
bool boardmap_is_board_id_exists(uint8_t id_board);
uint8_t boardmap_opendoor_key(void);
#endif

END_DECLS

#endif
