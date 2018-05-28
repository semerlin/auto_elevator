/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _KEYMAP_H_
  #define _KEYMAP_H_

#include "types.h"

BEGIN_DECLS

bool keymap_init(void);
uint8_t keymap_convert(int floor);

END_DECLS

#endif
