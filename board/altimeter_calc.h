/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _ALTIMETER_CALC_H_
#define _ALTIMETER_CALC_H_

#ifdef __MASTER
#include "types.h"

BEGIN_DECLS

typedef enum
{
    CALC_START = 0x01,
    CALC_STOP = 0x02,
} calc_action_t;


bool altimeter_calc_init(void);
bool altimeter_calc_run(calc_action_t action, char start_floor, char end_floor);
bool altimeter_is_calculating(void);
void altimeter_calc_once(char floor);
uint16_t alitmeter_floor_height(void);

END_DECLS
#endif

#endif /* _ALTIMETER_CALC_H_ */
