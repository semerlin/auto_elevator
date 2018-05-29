/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _FM24CL64_H_
  #define _FM24CL64_H_

#include "types.h"

BEGIN_DECLS

bool fm_init(void);
bool fm_write(uint16_t addr, const uint8_t *data, uint16_t len);
bool fm_read(uint16_t addr, uint8_t *data, uint16_t len);


END_DECLS

#endif
