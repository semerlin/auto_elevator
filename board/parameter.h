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

BEGIN_DECLS

typedef enum
{
    CALC_PWD,
    CALC_ALTIMETER,
} calc_type_t;


#ifdef __MASTER
#define PARAM_PWD_LEN            4
typedef struct
{
    uint8_t id_ctl;
    uint8_t id_elev;
    uint8_t id_board; /** fixed to 0x01 */
    char start_floor;
    uint8_t total_floor;
    uint8_t calc_type;
    uint8_t pwd_window;
    uint8_t pwd[PARAM_PWD_LEN];
    uint16_t floor_height;
} parameters_t;
#else
typedef struct
{
    uint8_t id_board; /** valid range is 0x02-0xff */
    char start_floor;
} parameters_t;
#endif


bool param_init(void);
void reset_param(void);
bool is_param_setted(void);
bool param_store(parameters_t param);
#ifdef __MASTER
bool param_store_pwd(uint8_t interval, uint8_t *pwd);
bool param_store_floor_height(uint16_t height);
#endif
parameters_t param_get(void);
void param_dump(void);


END_DECLS


#endif /* _BOARD_PARAMETERS_H_ */
