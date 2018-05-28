/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _I2C_SOFTWARE_H_
  #define _I2C_SOFTWARE_H_
#include "types.h"

BEGIN_DECLS

typedef enum
{
    i2c1,
    i2c2,
    port_count,
}i2c_port;

typedef struct _i2c_t i2c;

i2c *i2c_request(i2c_port port);
void i2c_release(i2c *pi2c);
void i2c_set_slaveaddr(i2c *pi2c, uint8_t address);
bool i2c_write(i2c *pi2c, const uint8_t *data, uint32_t length);
bool i2c_read(i2c *pi2c, uint8_t *data, uint32_t length);

END_DECLS

#endif