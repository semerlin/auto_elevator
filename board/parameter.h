/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

#include "types.h"
#include "config.h"

BEGIN_DECLS

typedef enum
{
    CALC_PWD,
    CALC_ALTIMETER,
} calc_type_t;


#ifdef __MASTER
typedef struct
{
    uint8_t floor;
    uint16_t height;
} floor_height_t;

typedef struct
{
    uint8_t id_ctl;
    uint8_t id_elev;
    uint8_t id_board; /** fixed to 0x01 */
    uint8_t start_floor;
    uint8_t total_floor;
    uint16_t threshold;
    uint8_t calc_type;
    uint8_t opendoor_polar;
    uint8_t bt_name[BT_NAME_MAX_LEN + 1];
    uint8_t pwd_window;
    uint8_t pwd[PARAM_PWD_LEN];
    floor_height_t floor_height[MAX_FLOOR_NUM + MAX_EXPAND_FLOOR_NUM * (MAX_BOARD_NUM - 1)];
} parameters_t;

typedef struct
{
    uint8_t license[16];
    uint8_t run_time[16];
    uint8_t random[4];
} license_t;
#else
typedef struct
{
    uint8_t id_board; /** valid range is 0x02-0xff */
    uint8_t start_floor;
} parameters_t;
#endif

bool param_init(void);
void reset_param(void);
bool is_param_setted(void);
bool param_store(const parameters_t *param);
#ifdef __MASTER
bool param_store_pwd(uint8_t interval, uint8_t *pwd);
bool param_store_floor_height(uint8_t len, const floor_height_t *floor_height);
bool param_store_bt_name(uint8_t len, const uint8_t *name);
#endif
#if !USE_SIMPLE_LICENSE
void reset_license(void);
bool param_has_license(void);
license_t param_get_license(void);
bool param_set_license(const license_t *license);
#endif
parameters_t param_get(void);
void param_dump(void);


END_DECLS


#endif /* _BOARD_PARAMETERS_H_ */
