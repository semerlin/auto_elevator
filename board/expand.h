/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _EXPAND_H_
#define _EXPAND_H_

#include "types.h"

BEGIN_DECLS

bool expand_init(void);
void expand_send_data(const uint8_t *buf, uint8_t len);

#ifdef __EXPAND
bool is_expand_board_registered(void);
#endif

END_DECLS

#endif

