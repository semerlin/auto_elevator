/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _PROTOCOL_ROBOT_H_
#define _PROTOCOL_ROBOT_H_

#ifdef __MASTER
#include "types.h"

BEGIN_DECLS

typedef enum
{
    ROBOT_WN,
    ROBOT_BT
} robot_wn_type_t;

typedef void (*process_robot_cb)(const uint8_t *data, uint8_t len);
bool process_robot_data(const uint8_t *data, uint8_t len, void *pargs);
void notify_arrive(uint8_t floor, void *pargs);
void register_arrive_cb(process_robot_cb cb);

END_DECLS
#endif


#endif /* _PROTOCOL_ROBOT_H_ */


