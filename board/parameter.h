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
void reset_param(void);
bool is_param_setted(void);
void param_set_inited(void);
void param_get_pwd(uint8_t *pwd);
uint8_t param_get_id_ctl(void);
uint8_t param_get_id_elev(void);
char param_get_floormap(void);
bool param_update_floormap(char floor);
bool param_update_id_ctl(uint8_t id);
bool param_update_id_elev(uint8_t id);
bool param_update_pwd(const uint8_t *pwd);
bool param_update_all(const uint8_t *data);





END_DECLS


#endif
