#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32f10x_cfg.h"
#include "global.h"
#include "i2c.h"
#include "environment.h"


/* bh1750 definition */
#define BH1750_ADDRESS      (0x23)
#define BH1750_SPEED        (100000)

/* register */
#define BH1750_POWER_DOWN      (0x00)
#define BH1750_POWER_ON        (0x01)
#define BH1750_RESET           (0x07)
#define BH1750_CON_HR1         (0x10)
#define BH1750_CON_HR2         (0x11)
#define BH1750_CON_LR          (0x13)
#define BH1750_ONE_HR1         (0x20)
#define BH1750_ONE_HR2         (0x21)
#define BH1750_ONE_LR          (0x23)


/**
* @brief init bh1750
* @param i2c handle
*/
void bh1750Init(i2c *pi2c)
{
    pi2c->device->i2c_setspeed(pi2c->i2c_handle, BH1750_SPEED);
    pi2c->device->i2c_setslaveaddress(pi2c->i2c_handle, BH1750_ADDRESS);
    pi2c->device->i2c_open(pi2c->i2c_handle);
}

/**
 * @brief start measure light
 * @param i2c handle
 */
void bh1750Start(i2c *pi2c)
{
    uint8_t data = BH1750_POWER_ON;
    pi2c->device->i2c_write(pi2c->i2c_handle, (const char *)&data, 1);
    data = BH1750_RESET;
    pi2c->device->i2c_write(pi2c->i2c_handle, (const char *)&data, 1);
    data = BH1750_ONE_HR1;
    pi2c->device->i2c_write(pi2c->i2c_handle, (const char *)&data, 1);
}

/**
 * @brief get light value
 * @param i2c module
 */
uint32_t bh1750GetLight(i2c *pi2c)
{
    uint16_t result = 0;
    uint8_t data[2] = {0, 0};
    pi2c->device->i2c_read(pi2c->i2c_handle, (char *)data, 2);
    result = data[0];
    result <<= 16;
    result |= data[1];
    return (uint32_t)(result / 1.2);
}