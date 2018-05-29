/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include <string.h>
#include "FreeRTOS.h"
#include "i2c_software.h"
#include "delay.h"
#include "stm32f10x_cfg.h"


/* i2c handle definition */
struct _i2c_t
{
    i2c_port port;
    uint8_t slave_addr;
    GPIO_Group sdapin_group;
    GPIO_Group sclpin_group;
    uint8_t sdapin_port;
    uint8_t sclpin_port;
};

/* pin operation */
static __INLINE void SDA_H(i2c *pi2c)
{
    GPIO_SetPin(pi2c->sdapin_group, pi2c->sdapin_port);
}

static __INLINE void SDA_L(i2c *pi2c)
{
    GPIO_ResetPin(pi2c->sdapin_group, pi2c->sdapin_port);
}

static __INLINE void SCL_H(i2c *pi2c)
{
    GPIO_SetPin(pi2c->sclpin_group, pi2c->sclpin_group);
}

static __INLINE void SCL_L(i2c * pi2c)
{
    GPIO_ResetPin(pi2c->sclpin_group, pi2c->sclpin_group);
}

static __INLINE uint8_t SDA_VAL(i2c *pi2c)
{
    return GPIO_ReadPin(pi2c->sdapin_group, pi2c->sdapin_port);
}


static void i2c_start(i2c *pi2c)
{
	SDA_H(pi2c);
	SCL_H(pi2c);
	delay_us(4);
	SDA_L(pi2c);
	delay_us(4);
	SCL_L(pi2c);
}

static void i2c_stop(i2c *pi2c)
{
	SCL_L(pi2c);
	SDA_L(pi2c);
	delay_us(4);
	SCL_H(pi2c);
	delay_us(4);
	SDA_H(pi2c);
	delay_us(4);
}

static void i2c_ack(i2c *pi2c)
{
	SCL_L(pi2c);
	SDA_L(pi2c);
    delay_us(2);
	SCL_H(pi2c);
    delay_us(2);
	SCL_L(pi2c);
}

static void i2c_nack(i2c *pi2c)
{
	SCL_L(pi2c);
	SDA_H(pi2c);
    delay_us(2);
	SCL_H(pi2c);
	delay_us(2);
	SCL_L(pi2c);
}

static uint8_t i2c_waitack(i2c *pi2c)
{
	uint8_t errCnt = 0;
	SDA_H(pi2c);
	delay_us(1);
	SCL_H(pi2c);
	delay_us(1);
    

	while(SDA_VAL(pi2c))
	{
		errCnt++;
		if(errCnt > 200)
		{
			i2c_stop(pi2c);
			return 1;
		}
	}

	SCL_L(pi2c);
	return 0;
}

static void _i2c_write(i2c *pi2c, uint8_t data)
{
	SCL_L(pi2c);
	for(uint8_t i = 0; i < 8; ++i)
	{
		if(0 != (data & 0x80))
        {
			SDA_H(pi2c);
        }
		else
        {
			SDA_L(pi2c);
        }
		data <<= 1;
        delay_us(2);
		SCL_H(pi2c);
        delay_us(2);
		SCL_L(pi2c);
        delay_us(2);
	}
}

static uint8_t _i2c_read(i2c *pi2c, bool ack)
{
	uint8_t data = 0;
	for(uint8_t i = 0; i < 8; ++i)
	{
		SCL_L(pi2c);
        SDA_H(pi2c);
        delay_us(2);
		SCL_H(pi2c);
		data <<= 1;
		if(SDA_VAL(pi2c))
			data++;
        delay_us(2);
	}

	if(ack)
    {
		i2c_ack(pi2c);
    }
	else
    {
		i2c_nack(pi2c);
    }
	
	return data;
}

/**
 * @brief request i2c device
 * @param i2c port
 */
i2c *i2c_request(i2c_port port)
{
    assert_param(port < port_count);
    i2c *pi2c = pvPortMalloc(sizeof(i2c) / sizeof(char));
    if(pi2c == NULL)
    {
        return NULL;
    }
    pi2c->port = port;
    pi2c->slave_addr = 0xff;
    if(port == i2c1)
    {
        pi2c->sclpin_group = GPIOB;
        pi2c->sdapin_group = GPIOB;
        pi2c->sclpin_port = 6;
        pi2c->sdapin_port = 7;
    }
    else
    {
        pi2c->sclpin_group = GPIOB;
        pi2c->sdapin_group = GPIOB;
        pi2c->sclpin_port = 10;
        pi2c->sdapin_port = 11;
    }
    return pi2c;
}


void i2c_release(i2c *pi2c)
{
    assert_param(pi2c != NULL);  
    vPortFree(pi2c);
}

/**
* @brief set slave address
* @param i2c handle
* @param slave address
*/
void i2c_set_slaveaddr(i2c *pi2c, uint8_t address)
{
    assert_param(pi2c != NULL);
    pi2c->slave_addr = address;
}

/**
* @brief write data to iic bus
* @param pi2c - i2c handler
* @param datra - data to write
* @param length - data length
*/
bool i2c_write(i2c *pi2c, const uint8_t *data, uint32_t length)
{
    assert_param(pi2c != NULL);
    /* start transfer */
    i2c_start(pi2c);
    /* send address */
    _i2c_write(pi2c, pi2c->slave_addr << 1);
    if(i2c_waitack(pi2c) != 0)
    {
        i2c_stop(pi2c);
        return FALSE;
    }
    /* send data */
    const uint8_t *pData = data;
    while(length--)
    {
        _i2c_write(pi2c, *pData);
        if(i2c_waitack(pi2c) != 0)
        {
            i2c_stop(pi2c);
            return FALSE;
        }
        pData++;
    }
    /* stop transfer */
    i2c_stop(pi2c);
    
    return TRUE;
}

/**
* @brief write data to iic bus, support address write
* @param pi2c - i2c handler
* @param addr - address to write
* @param addr_len - address length
* @param data - data to write
* @param data_len - data length
*/

bool i2c_addr_write(i2c *pi2c, const uint8_t *addr, uint8_t addr_len,
        const uint8_t *data, uint32_t data_len)
{
    assert_param(pi2c != NULL);
    /* start transfer */
    i2c_start(pi2c);
    /* send address */
    _i2c_write(pi2c, pi2c->slave_addr << 1);
    if (i2c_waitack(pi2c) != 0)
    {
        i2c_stop(pi2c);
        return FALSE;
    }
    /* send address */
    const uint8_t *pData = addr;
    while (addr_len --)
    {
        _i2c_write(pi2c, *pData);
        if (0 != i2c_waitack(pi2c))
        {
            i2c_stop(pi2c);
            return FALSE;
        }
        pData++;
    }
    /* send data */
    pData = data;
    while(data_len--)
    {
        _i2c_write(pi2c, *pData);
        if(0 != i2c_waitack(pi2c))
        {
            i2c_stop(pi2c);
            return FALSE;
        }
        pData++;
    }
    /* stop transfer */
    i2c_stop(pi2c);
    
    return TRUE;

}



/**
 * @brief read data from i2c bus
 * @param i2c handle
 * @param data buffer
 * @param data length
 */
bool i2c_read(i2c *pi2c, uint8_t *data, uint32_t length)
{
    assert_param(pi2c != NULL);
    /* start transfer */
    i2c_start(pi2c);
    /* send address */
    _i2c_write(pi2c, (pi2c->slave_addr << 1) | 0x01);
    if(i2c_waitack(pi2c) != 0)
    {
        i2c_stop(pi2c);
        return FALSE;
    }
    /* read data */
    uint8_t *pData = data;
    if(length == 0)
    {
        i2c_stop(pi2c);
    }
    else
    {
        length -= 1;
        while(length--)
        {
            *pData++ = _i2c_read(pi2c, TRUE);
        }
        *pData++ = _i2c_read(pi2c, FALSE);
        /* stop transfer */
        i2c_stop(pi2c);
    }
    
    return TRUE;
}
