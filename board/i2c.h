/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _I2C_H_
  #define _I2C_H_

#include "FreeRTOS.h"
#include "types.h"

BEGIN_DECLS

/* serial parameter definition */
typedef enum
{ 
	i2c1,
    i2c2,
    Port_Count,
}i2c_port;


typedef struct
{
    int8_t name[16];
    void* (*i2c_request)(i2c_port port);
    bool (*i2c_open)(void *handle);
    void (*i2c_close)(void *handle);
    void (*i2c_setspeed)(void *handle, uint32_t speed);
    void (*i2c_setslaveaddress)(void *handle, uint8_t address);
    void (*i2c_setbufferlength)(void *handle, UBaseType_t rxLen, 
                            UBaseType_t txLen);
    bool (*i2c_write)(void *handle, const uint8_t *data, 
                      uint32_t length);
    bool (*i2c_read)(void *handle, uint8_t *data, uint32_t length);
}i2c_device;

typedef struct
{
    void *i2c_handle;
    i2c_device *device;
}i2c;


/* interface */
void i2c_register_device(i2c_device *device);
i2c_device *i2c_get_device(const char *name);
void i2c_remove_device(const char *name);

END_DECLS

#endif

