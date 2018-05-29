/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifndef _PARAMETER_H_
  #define _PARAMETER_H_

#include "types.h"

BEGIN_DECLS

bool patam_init(void);
bool is_param_setted(void);
bool param_get_keymap(uint8_t *map);
bool param_get_pwd(uint8_t *pwd);
bool param_update_keymap(uint8_t *map);
bool param_update_pwd(uint8_t *pwd);

END_DECLS


#endif
