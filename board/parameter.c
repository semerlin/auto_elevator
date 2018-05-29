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

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[PARAM]"

/* address and length */
#define SET_ADDR   0x00
#define SET_FLAG   "INIT"
#define SET_LEN    4

#define MAP_ADDR   SET_ADDR + SET_LEN
#define MAP_LEN    31

#define PWD_ADDR   MAP_ADDR + MAP_LEN
#define PWD_LEN    4

bool param_setted = FALSE;


/**
 * @brief initialize parameter module
 * @return init status
 */
bool param_init(void)
{
    if (fm_init())
    {
        uint8_t status[SET_LEN];
        fm_read(SET_ADDR, status, SET_LEN);
        param_setted = (0 == strncmp((const char *)status, SET_FLAG, 4));
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

/**
 * @brief get key map
 * @param map - key map
 */
bool param_get_keymap(uint8_t *map)
{
    return fm_read(MAP_ADDR, map, MAP_LEN);
}

/**
 * @param get password
 * @param pwd - password
 */
bool param_get_pwd(uint8_t *pwd)
{
    return fm_read(PWD_ADDR, pwd, PWD_LEN);
}

/**
 * @brief update key map
 * @param map - key map
 * @return update status
 */
bool param_update_keymap(uint8_t *map)
{
    return fm_write(MAP_ADDR, map, MAP_LEN);
}

/**
 * @brief update password
 * @param pwd - password 
 * @return update status
 */
bool param_update_pwd(uint8_t *pwd)
{
    return fm_write(PWD_ADDR, pwd, PWD_LEN);
}


