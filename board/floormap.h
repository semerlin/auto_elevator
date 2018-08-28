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
uint8_t floormap_dis_to_phy(char floor);
char floormap_phy_to_dis(uint8_t floor);
bool floormap_contains_floor(char floor);

END_DECLS
#endif

#endif  /* _FLOORMAP_H_ */