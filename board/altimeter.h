/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _ALTIMETER_H_
#define _ALTIMETER_H_

#ifdef __MASTER
#include "types.h"

BEGIN_DECLS

bool altimeter_init(void);
uint32_t altimeter_get_distance(void);

END_DECLS

#endif

#endif /* _ALTIMETER_H_*/
