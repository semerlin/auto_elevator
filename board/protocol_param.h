/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _PROTOCOL_PARAM_H_
#define _PROTOCOL_PARAM_H_

#include "types.h"

BEGIN_DECLS

bool process_param_data(const uint8_t *data, uint8_t len, void *args);
#ifdef __MASTER
void notify_calc(uint16_t floor_height, uint16_t distance);
#endif

END_DECLS


#endif /* _PROTOCOL_PARAM_H_ */


