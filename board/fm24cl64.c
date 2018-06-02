/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "fm24cl64.h"
#include "i2c_software.h"
#include "assert.h"
#include "trace.h"
#include "pinconfig.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[FM]"

/* FM24CL64 i2c address */
#define FM24CL64_ADDRESS 0x50

static i2c *fm_i2c = NULL;

/**
 * @brief set fm write protect
 * @param flag - wp enable or disable
 */
static __INLINE void fm_wp(bool flag)
{
    if (flag)
    {
        pin_set("FM_WP");
    }
    else
    {
        pin_reset("FM_WP");
    }
}
/**
 * @brief initialize fm device
 * @return init status
 */
bool fm_init(void)
{
    TRACE("initialize fm24cl64...\r\n");
    fm_i2c = i2c_request(i2c1);
    if (NULL == fm_i2c)
    {
        TRACE("initialize fm24cl64 failed!\r\n");
        return FALSE;
    }
    i2c_set_slaveaddr(fm_i2c, FM24CL64_ADDRESS);
    return TRUE;
}

/**
 * @brief write data to fm24
 * @param addr - address to write
 * @param data - data to write
 * @param len - data length
 */
bool fm_write(uint16_t addr, const uint8_t *data, uint16_t len)
{
    assert_param(NULL != fm_i2c);
    TRACE("write %d datas to 0x%x\r\n", len, addr);
    uint8_t addr_data[2];
    addr_data[0] = (uint8_t)((addr >> 8) & 0xff);
    addr_data[1] = (uint8_t)(addr & 0xff);
    fm_wp(FALSE);
    bool ret = i2c_addr_write(fm_i2c, addr_data, 2, data, len);
    fm_wp(TRUE);
    
    return ret;
}

/**
 * @brief read data from fm24
 * @param addr - address to read 
 * @param data - data to read 
 * @param len - data length
 */
bool fm_read(uint16_t addr, uint8_t *data, uint16_t len)
{
    assert_param(NULL != fm_i2c);
    TRACE("read %d datas from 0x%x\r\n", len, addr);
    uint8_t addr_data[2];
    addr_data[0] = (uint8_t)((addr >> 8) & 0xff);
    addr_data[1] = (uint8_t)(addr & 0xff);
    if (i2c_write(fm_i2c, addr_data, 2))
    {
        return i2c_read(fm_i2c, data, len);
    }

    return FALSE;
}


