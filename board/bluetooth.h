/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifdef __MASTER
#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#include "types.h"

BEGIN_DECLS

bool bt_init(void);
bool bt_set_name(const char *name);
bool bt_is_connected(void);

END_DECLS


#endif
#endif

