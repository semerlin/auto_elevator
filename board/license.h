/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _LICENSE_H_
#define _LICENSE_H_

#include "types.h"
#include "config.h"

BEGIN_DECLS

bool license_init(void);
#if !USE_SIMPLE_LICENSE
void license_generate_key(const uint8_t *chip_id, uint8_t *pkey);
void key_to_serial_number(const uint8_t *pkey, uint8_t *pserial);
void encrypt_time(uint64_t time, const uint8_t *pserial, uint8_t *pencrypt_time);
bool decrypt_time(const uint8_t *pencrypt_time, const uint8_t *pserial, uint64_t *ptime);
#endif

END_DECLS

#endif
