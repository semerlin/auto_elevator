/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _LED_MONITOR_H_
#define _LED_MONITOR_H_

#include "types.h"

BEGIN_DECLS

bool led_monitor_init(void);
#ifdef __MASTER
void led_monitor_process(uint8_t id_board, uint16_t prev_status, uint16_t cur_status);
#endif

END_DECLS

#endif /* _LED_MONITOR_H_ */
