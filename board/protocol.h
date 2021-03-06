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
#include "protocol_param.h"
#include "protocol_robot.h"

BEGIN_DECLS

bool ptl_init(void);
void ptl_send_data(const uint8_t *data, uint8_t len);

END_DECLS


#endif /* _PROTOCOL_H_ */


