/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _PROTOCOL_H_
  #define _PROTOCOL_H_

#include "types.h"

BEGIN_DECLS

typedef void (*process_cb)(const uint8_t *data, uint8_t len);

bool protocol_init(void);
void notify_arrive(uint8_t floor);
void register_arrive_cb(process_cb cb);


END_DECLS


#endif /* _PROTOCOL_H_ */


