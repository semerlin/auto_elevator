/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _FLOORMAP_H_
#define _FLOORMAP_H_

#ifdef __MASTER
#include "types.h"

BEGIN_DECLS

void floormap_update(void);
bool floormap_contains_floor(uint8_t floor);

END_DECLS
#endif

#endif  /* _FLOORMAP_H_ */
