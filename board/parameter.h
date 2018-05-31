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

/**
 * @brief initialize parameter module
 * @return init status
 */
bool param_init(void);
bool is_param_setted(void);
void param_get_pwd(uint8_t *pwd);
bool param_update_pwd(uint8_t *pwd);
uint8_t param_get_id_ctl(void);
bool param_update_id_ctl(uint8_t id);
uint8_t param_get_id_elev(void);
bool param_update_id_elev(uint8_t id);
#ifdef _CFG_KEYMAP
void param_get_keymap(uint8_t *map);
bool param_update_keymap(uint8_t *map);
#endif





END_DECLS


#endif
