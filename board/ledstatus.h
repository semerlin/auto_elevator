/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _LEDSTATUS_H_
  #define _LEDSTATUS_H_

#include "types.h"

BEGIN_DECLS

uint16_t ledstatus_get(void);
bool is_ledon(uint8_t floor);
bool is_down_ledon(uint8_t floor);
bool is_up_ledon(uint8_t floor);
bool is_ledon_except(uint8_t floor);

END_DECLS


#endif /* _LED_MOTOR_H_ */

