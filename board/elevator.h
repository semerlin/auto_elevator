/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _ELEVATOR_H_
#define _ELEVATOR_H_

#include "types.h"

BEGIN_DECLS

typedef enum
{
    work_stop,
    work_idle,
    work_robot,
    work_full,
}elev_work_state;

typedef enum
{
    run_stop,
    run_up,
    run_down,
}elev_run_state;


bool elev_init(void);
void elev_arrived(char floor);
void elev_go(char floor);
void elev_hold_open(bool flag);
char elev_floor(void);
void elev_decrease(void);
void elev_increase(void);
void elev_set_first_floor(void);
elev_run_state elev_state_run(void);
elev_work_state elev_state_work(void);
void elevator_set_state_work(elev_work_state state);
void elev_key_reset();

END_DECLS


#endif /* _ELEVATOR_H_ */

