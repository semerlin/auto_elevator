/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _SWITCH_MONITOR_H_
#define _SWITCH_MONITOR_H_

#ifdef __MASTER
#include "types.h"

BEGIN_DECLS

typedef enum
{
    switch_arrive,
    switch_run,
} switch_status;


bool switch_monitor_init(void);
switch_status switch_get_status(void);

END_DECLS
#endif

#endif /* _SWITCH_MONITOR_H_ */

