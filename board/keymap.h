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

#define INVALID_FLOOR    125
#define INVALID_KEY      0xff

bool keymap_init(void);
uint8_t keymap_floor_to_key(char floor);
char keymap_key_to_floor(uint8_t key);
uint8_t keymap_open(void);
void keymap_update(const char *data);

END_DECLS

#endif
