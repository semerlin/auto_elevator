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
#define SET_ADDR   0
#define SET_FLAG   "INIT"
#define SET_LEN    4

#define FLOORMAP_ADDR       (SET_ADDR + SET_LEN)
#define FLOORMAP_LEN        1
static uint8_t param_floormap;

#define ID_CTL_ADDR    (FLOORMAP_ADDR + FLOORMAP_LEN)
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
 * @brief reset parameter
 */
void reset_param(void)
{
    uint8_t status[SET_LEN] = {0, 0, 0, 0};
    fm_write(SET_ADDR, status, SET_LEN);
}

/**
 * @brief initialize parameter module
 * @return init status
 */
bool param_init(void)
{
    TRACE("initialize fm...\r\n");
    if (fm_init())
    {
        //reset_param();
        uint8_t status[SET_LEN];
        if (!fm_read(SET_ADDR, status, SET_LEN))
        {
            return FALSE;
        }
        param_setted = (0 == strncmp((const char *)status, SET_FLAG, 4));
        if (param_setted)
        {
            TRACE("parameter setted, update it\r\n");
            if (!fm_read(FLOORMAP_ADDR, &param_floormap, FLOORMAP_LEN))
            {
                return FALSE;
            }
            if (!fm_read(ID_CTL_ADDR, &param_id_ctl, ID_CTL_LEN))
            {
                return FALSE;
            }
            if (!fm_read(ID_ELEV_ADDR, &param_id_elev, ID_ELEV_LEN))
            {
                return FALSE;
            }
            if (!fm_read(PWD_ADDR, param_pwd, PWD_LEN))
            {
                return FALSE;
            }
        }
        else
        {
            TRACE("parameter not setted\r\n");
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

/**
 * @brief get key map
 * @param map - key map
 */
char param_get_floormap(void)
{
    return (char )param_floormap;
}
/**
 * @brief update key map
 * @param map - key map
 * @return update status
 */
bool param_update_floormap(char floor)
{
    TRACE("update floor map\r\n");
    if ((uint8_t)floor != param_floormap)
    {
        param_floormap = (uint8_t)floor;
        return fm_write(FLOORMAP_ADDR, &param_floormap, FLOORMAP_LEN);
    }

    return TRUE;
}

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
    TRACE("update control id\r\n");
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
    TRACE("update elevator id\r\n");
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
bool param_update_pwd(const uint8_t *pwd)
{
    TRACE("update password\r\n");
    if (0 != strncmp((const char *)param_pwd, (const char *)pwd, PWD_LEN))
    {
        strncpy((char *)param_pwd, (const char *)pwd, PWD_LEN);
        return fm_write(PWD_ADDR, pwd, PWD_LEN);
    }

    return TRUE;
}

/**
 * @brief update all parameter data
 * @param data - data to update
 * @return update status
 */
bool param_update_all(const uint8_t *data)
{
    if (!fm_write(SET_ADDR, SET_FLAG, SET_LEN))
    {
        return FALSE;
    }
    if (!param_update_floormap(*data))
    {
        return FALSE;
    }
    if (!param_update_id_ctl(*(data + ID_CTL_ADDR - SET_LEN)))
    {
        return FALSE;
    }
    if (!param_update_id_elev(*(data + ID_ELEV_ADDR - SET_LEN)))
    {
        return FALSE;
    }
    if (!param_update_pwd(data + PWD_ADDR - SET_LEN))
    {
        return FALSE;
    }
    
    param_setted = TRUE;
    return TRUE;
}


