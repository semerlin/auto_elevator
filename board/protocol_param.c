/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include <string.h>
#include "protocol.h"
#include "protocol_param.h"
#include "trace.h"
#include "stm32f10x_cfg.h"
#include "parameter.h"
#include "crc.h"
#include "altimeter.h"
#include "altimeter_calc.h"
#include "protocol_expand.h"
#include "bluetooth.h"
#include "delay.h"
#include "license.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ptl_param]"

extern parameters_t board_parameter;
extern license_t license;

/* protocol head and tail */
#define PARAM_HEAD    0x55
#define PARAM_TAIL    0xaa

static void process_param_set(const uint8_t *data, uint8_t len);
#ifdef __MASTER
static void process_param_pwd(const uint8_t *data, uint8_t len);
static void process_param_calc(const uint8_t *data, uint8_t len);
static void process_param_bt_name(const uint8_t *data, uint8_t len);
#endif
static void process_reboot(const uint8_t *data, uint8_t len);
static void process_license(const uint8_t *data, uint8_t len);

typedef enum
{
    SUCCESS,
    CRC_ERROR,
    INVALID_PARAM,
    OPERATION_FAIL,
} param_status_t;

/* process handle */
typedef struct
{
    uint8_t cmd;
    void (*process)(const uint8_t *data, uint8_t len);
} cmd_handle_t;

/* protocol command */
#define CMD_SET            0x01
#ifdef __MASTER
#define CMD_PWD            0x02
#define CMD_CALC           0x03
#define CMD_BT_NAME        0x04
#define CMD_CALC_NOTIFY    0x80
#endif
#define CMD_REBOOT         0x05
#define CMD_LICENSE        0x06

static cmd_handle_t cmd_handles[] =
{
    {CMD_SET, process_param_set},
#ifdef __MASTER
    {CMD_PWD, process_param_pwd},
    {CMD_CALC, process_param_calc},
    {CMD_BT_NAME, process_param_bt_name},
#endif
    {CMD_REBOOT, process_reboot},
    {CMD_LICENSE, process_license},
};

typedef struct
{
    /**
     * 0x01: master and expand board
     */
    uint8_t reboot_type;
} msg_reboot_t;

typedef struct
{
    uint8_t license[16];
} msg_license_t;

#define IS_FLOOR_VALID(floor)               (floor > 0)

#ifdef __MASTER

#define IS_PWD_SCAN_WINDOW_VALID(window)    (0x00 != window)
#define IS_CALC_TYPE_VALID(type)            ((CALC_PWD == (type)) || (CALC_ALTIMETER == (type)))
#define IS_ACTION_VALID(action)             ((0x01 == action) || (0x02 == action))
#define IS_TOTAL_FLOOR_VALID(floor)         (floor > 0)
#define IS_BT_NAME_LEN_VALID(len)           ((len > 0) && (len <= BT_NAME_MAX_LEN))
#define IS_REBOOT_VALID(reboot)             (0x01 == reboot)

#pragma pack(1)
typedef struct
{
    uint8_t id_ctl;
    uint8_t id_elev;
    uint8_t start_floor;
    uint8_t total_floor;
    uint16_t threshold;
    calc_type_t calc_type;
    uint8_t opendoor_polar;
} msg_param_t;

typedef struct
{
    uint8_t scan_window;
    uint8_t pwd[4];
} msg_pwd_t;

typedef struct
{
    uint8_t action; /** 0x01: start 0x02: stop */
} msg_calc_t;

typedef struct
{
    uint8_t bt_name[0];
} msg_bt_name_t;

typedef struct
{
    uint8_t opcode;
    uint8_t floor;
    uint16_t floor_height;
    uint16_t distance;
} msg_calc_data;
#pragma pack()

#else

#pragma pack(1)
typedef struct
{
    /** valid range id 0x02-0xfe */
    uint8_t id_board;
    uint8_t start_floor;
} msg_param_t;
#pragma pack()

#define IS_BOARD_ID_VALID(id) (((id) >= 0x02) && ((id) <= 0xfe))
#define IS_REBOOT_VALID(reboot) (0x01 == reboot)

#endif


/**
 * @brief replay to init
 * @param[] flag - init status
 */
static void param_reply(uint8_t cmd, uint8_t status)
{
    uint8_t rsp[7];
    uint16_t crc = crc16(&status, 1);
    rsp[0] = PARAM_HEAD;
    rsp[1] = 7;
    rsp[2] = cmd;
    rsp[3] = status;
    rsp[4] = (uint8_t)((crc >> 8) & 0xff);
    rsp[5] = (uint8_t)(crc & 0xff);
    rsp[6] = PARAM_TAIL;

    ptl_send_data(rsp, 7);
}

/**
 * @brief analyze protocol data
 * @param data - data to analyze
 * @param len - data length
 */
bool process_param_data(const uint8_t *data, uint8_t len, void *args)
{
    UNUSED(args);
    if (PARAM_HEAD != data[0])
    {
        return FALSE;
    }

    /* check length and tail */
    uint8_t pkt_len = data[1];
    if ((pkt_len <= len) && (PARAM_TAIL == data[pkt_len - 1]))
    {
        /* check crc value */
        uint16_t recv_crc = data[len - 3];
        recv_crc <<= 8;
        recv_crc |= data[len - 2];
        uint16_t calc_crc = 0;
        calc_crc = crc16(data + 2, pkt_len - 5);
        if (recv_crc == calc_crc)
        {
            for (int i = 0; i < sizeof(cmd_handles) / sizeof(cmd_handles[0]); ++i)
            {
                if (data[2] == cmd_handles[i].cmd)
                {
                    cmd_handles[i].process(data + 3, len - 6);
                    break;
                }
            }
        }
        else
        {
            param_reply(CMD_SET, CRC_ERROR);
        }
    }

    return TRUE;
}

/**
 * @brief process parameter set
 * @param data - parameter
 * @param len - parameter length
 */
static void process_param_set(const uint8_t *data, uint8_t len)
{
    param_status_t status = SUCCESS;
    if (len == sizeof(msg_param_t))
    {
        msg_param_t *msg = (msg_param_t *)data;

        if (!IS_FLOOR_VALID(msg->start_floor))
        {
            status = INVALID_PARAM;
            goto END;
        }
        board_parameter.start_floor = msg->start_floor;
#ifdef __MASTER
        if ((!IS_TOTAL_FLOOR_VALID(msg->total_floor)) ||
            (!IS_CALC_TYPE_VALID(msg->calc_type)))
        {
            status = INVALID_PARAM;
            goto END;
        }

        board_parameter.id_ctl = msg->id_ctl;
        board_parameter.id_elev = msg->id_elev;
        board_parameter.total_floor = msg->total_floor;
        board_parameter.threshold = msg->threshold;
        board_parameter.calc_type = msg->calc_type;
        board_parameter.opendoor_polar = msg->opendoor_polar;
        board_parameter.id_board = 0x01;
#endif

#ifdef __EXPAND
        if (!IS_BOARD_ID_VALID(msg->id_board))
        {
            status = INVALID_PARAM;
            goto END;
        }
        board_parameter.id_board = msg->id_board;
#endif
        if (!param_store(&board_parameter))
        {
            status = OPERATION_FAIL;
        }
    }
    else
    {
        status = OPERATION_FAIL;
    }

END:
    param_reply(CMD_SET, status);
    if (SUCCESS == status)
    {
        TRACE("rebooting...\r\n");
        SCB_SystemReset();
    }
}

/**
 * @brief process reboot
 * @param data - calculation data
 * @param len - data length
 */
static void process_reboot(const uint8_t *data, uint8_t len)
{
    param_status_t status = SUCCESS;
    if (len == sizeof(msg_reboot_t))
    {
        msg_reboot_t *pdata = (msg_reboot_t *)data;
        if (!IS_REBOOT_VALID(pdata->reboot_type))
        {
            status = INVALID_PARAM;
        }
    }
    else
    {
        status = OPERATION_FAIL;
    }
    param_reply(CMD_REBOOT, status);
#ifdef __MASTER

    switch (data[0])
    {
    case 0x01:
        /** notify all expand board */
        expand_reboot_immediately(0xff);
        delay_ms(500);
        SCB_SystemReset();
        break;
    default:
        break;
    }
#else
    SCB_SystemReset();
#endif
}

/**
 * @brief process license
 * @param data - calculation data
 * @param len - data length
 */
static void process_license(const uint8_t *data, uint8_t len)
{
    param_status_t status = SUCCESS;
    if (len >= sizeof(msg_license_t))
    {
        msg_license_t *pdata = (msg_license_t *)data;
        if (!license_set(pdata->license))
        {
            status = OPERATION_FAIL;
        }
    }
    else
    {
        status = OPERATION_FAIL;
    }
    param_reply(CMD_LICENSE, status);
    if (SUCCESS == status)
    {
        SCB_SystemReset();
    }
}

#ifdef __MASTER
/**
 * @brief process password set
 * @param data - password
 * @param len - parameter length
 */
static void process_param_pwd(const uint8_t *data, uint8_t len)
{
    param_status_t status = SUCCESS;
    if (CALC_PWD == board_parameter.calc_type)
    {
        if (len == sizeof(msg_pwd_t))
        {
            msg_pwd_t *msg = (msg_pwd_t *)data;
            if (IS_PWD_SCAN_WINDOW_VALID(msg->scan_window))
            {
                if (!param_store_pwd(msg->scan_window, msg->pwd))
                {
                    status = OPERATION_FAIL;
                }
            }
            else
            {
                status = INVALID_PARAM;
            }
        }
        else
        {
            status = OPERATION_FAIL;
        }
    }
    else
    {
        status = INVALID_PARAM;
    }

    param_reply(CMD_PWD, status);
    if (SUCCESS == status)
    {
        TRACE("rebooting...\r\n");
        SCB_SystemReset();
    }
}

/**
 * @brief process calculation set
 * @param data - calculation data
 * @param len - data length
 */
static void process_param_calc(const uint8_t *data, uint8_t len)
{
    param_status_t status = SUCCESS;
    if (CALC_ALTIMETER == board_parameter.calc_type)
    {
        if (len == sizeof(msg_calc_t))
        {
            msg_calc_t *calc = (msg_calc_t *)data;
            if (!IS_ACTION_VALID(calc->action))
            {
                status = INVALID_PARAM;
            }
        }
        else
        {
            status = OPERATION_FAIL;
        }
    }
    else
    {
        status = INVALID_PARAM;
    }

    param_reply(CMD_CALC, status);

    if (SUCCESS == status)
    {
        msg_calc_t *calc = (msg_calc_t *)data;
        altimeter_calc_run((calc_action_t)(calc->action));
    }
}

/**
 * @brief process bluetooth name
 * @param data - bluetooth name
 * @param len - data length
 */
static void process_param_bt_name(const uint8_t *data, uint8_t len)
{
    param_status_t status = SUCCESS;
    uint8_t bt_name[BT_NAME_MAX_LEN + 1];
    if (IS_BT_NAME_LEN_VALID(len))
    {
        memset(bt_name, 0, BT_NAME_MAX_LEN + 1);
        memcpy(bt_name, data, len);
        if (!param_store_bt_name(len, bt_name))
        {
            status = OPERATION_FAIL;
        }
    }
    else
    {
        status = INVALID_PARAM;
    }

    param_reply(CMD_BT_NAME, status);
    if (SUCCESS == status)
    {
        bt_set_name((const char *)bt_name);
    }
}

/**
 * @brief notify calculation data to user
 */
void notify_calc(uint8_t floor, uint16_t floor_height, uint16_t distance)
{
    msg_calc_data msg;
    msg.opcode = CMD_CALC_NOTIFY;
    msg.floor_height = floor_height;
    msg.distance = distance;
    uint8_t rsp[13];
    uint16_t crc = crc16((uint8_t *)&msg, sizeof(msg));
    rsp[0] = PARAM_HEAD;
    rsp[1] = 5 + sizeof(msg);
    rsp[2] = msg.opcode;
    rsp[3] = floor;
    rsp[4] = (uint8_t)(msg.floor_height >> 8);
    rsp[5] = (uint8_t)(msg.floor_height & 0xff);
    rsp[6] = (uint8_t)(msg.distance >> 8);
    rsp[7] = (uint8_t)(msg.distance & 0xff);
    rsp[8] = (uint8_t)((crc >> 8) & 0xff);
    rsp[9] = (uint8_t)(crc & 0xff);
    rsp[10] = PARAM_TAIL;

    ptl_send_data(rsp, 5 + sizeof(msg));
}
#endif
