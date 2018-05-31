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

#ifdef _CFG_KEYMAP
#define MAP_ADDR       (SET_ADDR + SET_LEN)
#define MAP_LEN        31
static uint8_t param_map[32];
#endif

#ifdef _CFG_KEYMAP
#define ID_CTL_ADDR    (MAP_ADDR + MAP_LEN)
#else
#define ID_CTL_ADDR    (SET_ADDR + SET_LEN)
#endif
#define ID_CTL_LEN     1
static uint8_t param_id_ctl;

#define ID_ELEV_ADDR    (ID_CTL_ADDR + ID_CTL_LEN)
#define ID_ELEV_LEN     1
static uint8_t param_id_elev;

#define PWD_ADDR        (ID_ELEV_ADDR + ID_ELEV_LEN)
#define PWD_LEN         4
static uint8_t param_pwd[5];

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
        if (param_setted)
        {
            #ifdef _CFG_KEYMAP
            fm_read(MAP_ADDR, param_map, MAP_LEN);
            #endif
            fm_read(ID_CTL_ADDR, &param_id_ctl, ID_CTL_LEN);
            fm_read(ID_ELEV_ADDR, &param_id_elev, ID_ELEV_LEN);
            fm_read(PWD_ADDR, param_pwd, PWD_LEN);
        }
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

#ifdef _CFG_KEYMAP
/**
 * @brief get key map
 * @param map - key map
 */
void param_get_keymap(uint8_t *map)
{
    for (int i = 0; i < MAP_LEN; ++i)
    {
        map[i] = param_map[i];
    }
}
/**
 * @brief update key map
 * @param map - key map
 * @return update status
 */
bool param_update_keymap(uint8_t *map)
{
    if (0 != strncmp((const char *)param_map, (const char *)map, MAP_LEN))
    {
        strncpy((char *)param_map, (const char *)map, MAP_LEN);
        return fm_write(MAP_ADDR, map, MAP_LEN);
    }

    return TRUE;
}
#endif

/**
 * @brief get password
 * @param pwd - password
 */
void param_get_pwd(uint8_t *pwd)
{
    for (int i = 0; i < PWD_LEN; ++i)
    {
        pwd[i] = param_pwd[i];
    }
}

/**
 * @brief get control id
 * @param id - id
 */
uint8_t param_get_id_ctl(void)
{
    return param_id_ctl;
}

/**
 * @brief get elevator id
 * @param id - id
 */
uint8_t param_get_id_elev(void)
{
    return param_id_elev;
}

/**
 * @brief update board id
 * @param id - board id
 */
bool param_update_id_ctl(uint8_t id)
{
    if (param_id_ctl != id)
    {
        param_id_ctl = id;
        return fm_write(ID_CTL_ADDR, &id, ID_CTL_LEN);
    }

    return TRUE;
}

/**
 * @brief update board id
 * @param id - board id
 */
bool param_update_id_elev(uint8_t id)
{
    if (param_id_elev != id)
    {
        param_id_elev = id;
        return fm_write(ID_ELEV_ADDR, &id, ID_ELEV_LEN);
    }

    return TRUE;
}

/**
 * @brief update password
 * @param pwd - password 
 * @return update status
 */
bool param_update_pwd(uint8_t *pwd)
{
    if (0 != strncmp((const char *)param_pwd, (const char *)pwd, PWD_LEN))
    {
        strncpy((char *)param_pwd, (const char *)pwd, PWD_LEN);
        return fm_write(PWD_ADDR, pwd, PWD_LEN);
    }

    return TRUE;
}


