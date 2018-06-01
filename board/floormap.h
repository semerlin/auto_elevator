/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _FLOORMAP_H_
  #define _FLOORMAP_H_

#include "types.h"

BEGIN_DECLS

uint8_t floormap_dis_to_phy(char floor);
char floormap_phy_to_dis(uint8_t floor);

END_DECLS

#endif  /* _FLOORMAP_H_ */