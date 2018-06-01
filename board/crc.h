/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _CRC_H_
#define _CRC_H_

#include "types.h"

BEGIN_DECLS

uint16_t crc16(const uint8_t *data, uint8_t len);

END_DECLS

#endif
