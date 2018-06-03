/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _ROBOT_H_
  #define _ROBOT_H_

#include "types.h"

BEGIN_DECLS

#define DEFAULT_CHECKIN   0xf7

void robot_init(void);
bool robot_checkin_set(uint8_t id, uint8_t floor);
void robot_checkin_reset(uint8_t id);
uint8_t robot_checkin_get(uint8_t id);
uint8_t robot_id_get(uint8_t floor);
bool robot_is_checkin(uint8_t floor);
uint8_t robot_checkin_cur();

END_DECLS


#endif /* _ROBOT_H_ */

