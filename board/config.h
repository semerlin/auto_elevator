/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define CAN_REMAP               0

#define SIMPLE_FILTER           0
#define AUTO_ADJUST             1

#define DUMP_ALTIMETER_DATA     0
#define DUMP_FLOORMAP           1
#define DUMP_BOARDMAP           1
#define DUMP_PROTOCOL           1
#define DUMP_BT                 1
#define DUMP_EXPAND             1

#define LOOP_BACK_TEST          0
#define USE_SPEED_100K          1

#ifdef __MASTER
#define MAX_FLOOR_NUM           15
#define MAX_EXPAND_FLOOR_NUM    16
#define MAX_BOARD_NUM           6
#define START_KEY               1
#define EXPAND_START_KEY        0
#define PARAM_PWD_LEN           4
#define BT_NAME_MAX_LEN         16
#else
#define MAX_BOARD_NUM           1
#define MAX_FLOOR_NUM           16
#define START_KEY               0
#endif

#define INVALID_FLOOR           0
#define INVALID_KEY             0xff

#define ID_BOARD_MASTER         0x01
#define ID_BOARD_INVALID        0xff

#endif /** _CONFIG_H_ */
