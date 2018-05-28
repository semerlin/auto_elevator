/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "i2c.h"
#include <string.h>
#include "FreeRTOS.h"

#define MAX_LIST_NUM   (5)

static i2c_device *device_list[MAX_LIST_NUM] = {NULL, NULL, NULL, NULL, NULL};


/**
 * @brief register i2c device
 * @param device description
 */
void i2c_register_device(i2c_device *device)
{
    for (uint8_t i = 0; i < MAX_LIST_NUM; ++i)
    {
        if (device_list[i] == NULL)
        {
            device_list[i] = pvPortMalloc(sizeof(i2c_device) / sizeof(char));
            strcpy((char *)(device_list[i]->name), (const char *)(device->name));
            device_list[i]->i2c_request = device->i2c_request;
            device_list[i]->i2c_open = device->i2c_open;
            device_list[i]->i2c_close = device->i2c_close;
            device_list[i]->i2c_setspeed = device->i2c_setspeed;
            device_list[i]->i2c_setslaveaddress = device->i2c_setslaveaddress;
            device_list[i]->i2c_setbufferlength = device->i2c_setbufferlength;
            device_list[i]->i2c_write = device->i2c_write;
            device_list[i]->i2c_read = device->i2c_read;
            break;
        }
    }
}

/**
 * @brief get i2c device
 * @param device name
 */
i2c_device *i2c_get_device(const char *name)
{
    for (uint8_t i = 0; i < MAX_LIST_NUM; ++i)
    {
        if (0 == strcmp((char *)(device_list[i]->name), (const char *)name))
        {
            return device_list[i];
        }
    }
    
    return NULL;
}

/**
 * @brief remove i2c device
 * @param device name
 */
void i2c_remove_device(const char *name)
{
    for (uint8_t i = 0; i < MAX_LIST_NUM; ++i)
    {
        if (0 == strcmp((char *)(device_list[i]->name), (const char *)name))
        {
            vPortFree(device_list[i]);
            device_list[i] = NULL;
        }
    }
}
