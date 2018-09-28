/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "stm32f10x_sig.h"
#include "stm32f10x_map.h"
#include "stm32f10x_cfg.h"

/**
 * @brief get chip id
 * @param id - chip id
 * @param len - id length
 */
void Get_ChipID(uint8_t *data, uint8_t *len)
{
    assert_param(NULL != data);
    uint32_t id[3];
    id[2] = *(uint32_t *)(0X1FFFF7E8);
    id[1] = *(uint32_t *)(0X1FFFF7EC);
    id[0] = *(uint32_t *)(0X1FFFF7F0);
    for (uint8_t i = 0; i < 3; ++i)
    {
        for (uint8_t j = 0; j < 4; ++j)
        {
            data[i * 4 + j] = id[i];
            id[i] >>= 8;
        }
    }
    *len = 12;
}
