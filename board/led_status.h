/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _LED_STATUS_H_
#define _LED_STATUS_H_

#include "types.h"

BEGIN_DECLS

uint16_t led_status_get(void);
#ifdef __MASTER
bool is_led_on(uint8_t floor);
bool is_down_led_on(uint8_t floor);
bool is_up_led_on(uint8_t floor);
#endif

END_DECLS


#endif /* _LED_STATUS_H_ */
