/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _PROTOCOL_EXPAND_H_
#define _PROTOCOL_EXPAND_H_

#include "types.h"

BEGIN_DECLS

bool process_expand_data(const uint8_t *data, uint8_t len);
#ifdef __MASTER
void expand_elev_go(uint8_t id_board, uint8_t floor);
void expand_reboot(uint8_t id_board);
#endif
#ifdef __EXPAND
typedef void (*register_cb_t)(uint8_t *data, uint8_t len);
void set_register_cb(register_cb_t register_cb);
void register_board(uint8_t id_board, uint8_t start_floor);
void notify_led_status(uint8_t id_board, uint16_t led_status);
#endif

END_DECLS


#endif /* _PROTOCOL_PARAM_H_ */


