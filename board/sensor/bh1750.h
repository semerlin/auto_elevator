#ifndef _BH1750_H_
#define _BH1750_H_

#include "types.h"
#include "i2c.h"

void bh1750Init(i2c *pi2c);
void bh1750Start(i2c *pi2c);
uint32_t bh1750GetLight(i2c *pi2c);

#endif

