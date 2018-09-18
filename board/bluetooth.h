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
void bt_send_data(const uint8_t *data, uint8_t len);
bool bt_set_name(const char *name);

END_DECLS


#endif
#endif

