/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include <string.h>
#include "parameter.h"
#include "assert.h"
#include "trace.h"
#include "fm24cl64.h"
#include "dbgserial.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[PARAM]"

/* address and length */
#define PARAM_START_ADDRESS      0
#define PARAM_SETTED_FLAG        "AUTO"
#define PARAM_SETTED_FLAG_LEN    4

typedef struct
{
    uint8_t flag[PARAM_SETTED_FLAG_LEN];
    parameters_t parameters;
} flash_map_t;

static flash_map_t flash_map;

static bool param_setted = FALSE;

/**
 * @brief reset parameter
 */
void reset_param(void)
{
    uint8_t status[PARAM_SETTED_FLAG_LEN];
    memset(status, 0xff, PARAM_SETTED_FLAG_LEN);
    fm_write(PARAM_START_ADDRESS, status, PARAM_SETTED_FLAG_LEN);
}

/**
 * @brief initialize parameter module
 * @return init status
 */
bool param_init(void)
{
    TRACE("initialize parameter...\r\n");
    if (fm_init())
    {
        if (!fm_read(PARAM_START_ADDRESS, (uint8_t *)&flash_map, sizeof(flash_map_t)))
        {
            return FALSE;
        }
        param_setted = (0 == memcmp(flash_map.flag, PARAM_SETTED_FLAG, PARAM_SETTED_FLAG_LEN));
#ifdef __MASTER
        flash_map.parameters.bt_name[BT_NAME_MAX_LEN] = '\0';
#endif
        TRACE("parameter status(%d)\r\n", param_setted);
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief check parameter initialize status
 * @return init status
 */
bool is_param_setted(void)
{
    return param_setted;
}

bool param_store(parameters_t param)
{
    flash_map_t flash_map;
    memcpy(flash_map.flag, PARAM_SETTED_FLAG, PARAM_SETTED_FLAG_LEN);
    flash_map.parameters = param;
    return fm_write(PARAM_START_ADDRESS, (uint8_t *)&flash_map, sizeof(flash_map_t));
}

#ifdef __MASTER
bool param_store_pwd(uint8_t interval, uint8_t *pwd)
{
    uint8_t buf[PARAM_PWD_LEN + 1] = {interval, pwd[0], pwd[1], pwd[2], pwd[3]};
    uint32_t offset = OFFSET_OF(flash_map_t, parameters.pwd_window);
    return fm_write(PARAM_START_ADDRESS + offset, buf, PARAM_PWD_LEN + 1);
}

bool param_store_floor_height(uint16_t height)
{
    uint32_t offset = OFFSET_OF(flash_map_t, parameters.floor_height);
    return fm_write(PARAM_START_ADDRESS + offset, (uint8_t *)&height, sizeof(uint16_t));
}

bool param_store_bt_name(uint8_t len, const uint8_t *name)
{
    uint8_t bt_name[BT_NAME_MAX_LEN + 1];
    memset(bt_name, 0, BT_NAME_MAX_LEN + 1);
    memcpy(bt_name, name, len);
    uint32_t offset = OFFSET_OF(flash_map_t, parameters.bt_name);
    return fm_write(PARAM_START_ADDRESS + offset, bt_name, len + 1);
}
#endif

parameters_t param_get(void)
{
    return flash_map.parameters;
}

void param_dump(void)
{
    TRACE("flash data: ");
    uint8_t *data = (uint8_t *)&flash_map;
    for (int i = 0; i < sizeof(flash_map_t); ++i)
    {
        dbg_putchar("0123456789abcdef"[data[i] >> 4]);
        dbg_putchar("0123456789abcdef"[data[i] & 0x0f]);
        dbg_putchar(' ');
    }
    dbg_putchar('\r');
    dbg_putchar('\n');
}
