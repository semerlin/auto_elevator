/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/

#include "delay.h"


/**
* @brief init delay system
*/
void delayInit(void)
{
}


/**
 * @brief delay us
 * @param us to delay
 */
void delay_us(uint16_t time)
{
    uint16_t i = 0;
    while(time--)
    {
        i = 10;
        while(i--);
    }
}

/**
 * @brief delay ms
 * @param ms to delay
 */
void delay_ms(uint16_t time)
{
    uint16_t i = 0;
    while(time--)
    {
        i = 12000;
        while(i--);
    }
}