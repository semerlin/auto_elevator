/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _FLOOR_CTL_H_
  #define _FLOOR_CTL_H_

#include "types.h"

BEGIN_DECLS

bool floor_init(void);
void floor_arrived(char floor);
void floor_hold_open(char floor);
uint8_t floor_prev_state(void);
void floor_decrease();
void floor_increase();
void floor_set_first(void);

END_DECLS

#endif
