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

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ptl_param]"

/* protocol head and tail */
#define PARAM_HEAD    0x55
#define PARAM_TAIL    0xaa

static void process_param_set(const uint8_t *data, uint8_t len);
#ifdef __MASTER
static void process_param_pwd(const uint8_t *data, uint8_t len);
static void process_param_calc(const uint8_t *data, uint8_t len);
#endif

/* process handle */
typedef struct
{
    uint8_t cmd;
    void (*process)(const uint8_t *data, uint8_t len);
} cmd_handle;

/* protocol command */
#define CMD_SET            0x01
#ifdef __MASTER
#define CMD_PWD            0x02
#define CMD_CALC           0x03
#define CMD_CALC_NOTIFY    0x04
#endif

static cmd_handle cmd_handles[] =
{
    {CMD_SET, process_param_set},
#ifdef __MASTER
    {CMD_PWD, process_param_pwd},
    {CMD_CALC, process_param_calc},
#endif
};

#ifdef __MASTER
typedef enum
{
    CALC_START = 0x01,
    CALC_STOP = 0x02,
} calc_action_t;

typedef struct
{
    uint8_t id_ctl;
    uint8_t id_elev;
    uint8_t id_board;
    char start_floor;
    uint8_t calc_type;
} __PACKED__ msg_param_t;

typedef struct
{
    uint8_t interval;
    uint8_t pwd[4];
} __PACKED__ msg_pwd_t;

typedef struct
{
    uint8_t action;
    uint8_t interval;
} __PACKED__ msg_calc_t;

typedef struct
{
    uint16_t floor_dis;
    uint16_t top_dis;
    uint8_t top_floor;
} __PACKED__ msg_calc_data;
#else
typedef struct
{
    uint8_t id_board;
    char start_floor;
} __PACKED__ msg_param_t;
#endif


/**
 * @brief replay to init
 * @param[] flag - init status
 */
static void param_reply(uint8_t cmd, bool flag)
{
    uint8_t rsp[7];
    uint8_t status = (flag ? 0x00 : 0x01);
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
bool process_param_data(const uint8_t *data, uint8_t len)
{
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
    if (len == sizeof(msg_param_t))
    {
        msg_param_t *msg = (msg_param_t *)data;

        parameters_t param = param_get();
#ifdef __MASTER
        param.id_ctl = msg->id_ctl;
        param.id_elev = msg->id_elev;
        param.calc_type = msg->calc_type;
#endif
        param.id_board = msg->id_board;
        param.start_floor = msg->start_floor;

        if (!param_store(param))
        {
            goto END;
        }

        param_reply(CMD_SET, TRUE);
        TRACE("rebooting...\r\n");
        SCB_SystemReset();
        return ;
    }

END:
    param_reply(CMD_SET, FALSE);
    TRACE("set parameter failed!\r\n");
}

#ifdef __MASTER
/**
 * @brief process password set
 * @param data - password
 * @param len - parameter length
 */
static void process_param_pwd(const uint8_t *data, uint8_t len)
{
    if (len == sizeof(msg_pwd_t))
    {
        msg_pwd_t *msg = (msg_pwd_t *)data;
        if (!param_store_pwd(msg->interval, msg->pwd))
        {
            param_reply(CMD_PWD, TRUE);
            TRACE("rebooting...\r\n");
            SCB_SystemReset();
            return ;
        }
    }

    param_reply(CMD_SET, FALSE);
    TRACE("set password failed!\r\n");
}

/**
 * @brief process calculation set
 * @param data - calculation data
 * @param len - data length
 */
static void process_param_calc(const uint8_t *data, uint8_t len)
{
    if (len == sizeof(msg_calc_t))
    {
        msg_calc_t *calc = (msg_calc_t *)data;
        if (CALC_START == calc->action)
        {
            /* start calculation */
        }
        else
        {
            /* stop calculation */
        }
        param_reply(CMD_CALC, FALSE);
    }
    else
    {
        param_reply(CMD_CALC, FALSE);
    }
}

/**
 * @brief notify calculation data to user
 */
void notify_calc(uint16_t floor_dis, uint16_t top_dis, uint8_t top_floor)
{
    msg_calc_data msg;
    msg.floor_dis = floor_dis;
    msg.top_dis = top_dis;
    msg.top_floor = top_floor;

    uint8_t rsp[15];
    uint16_t crc = crc16((uint8_t *)&msg, sizeof(msg));
    rsp[0] = PARAM_HEAD;
    rsp[1] = 6 + sizeof(msg);
    rsp[2] = CMD_CALC_NOTIFY;
    memcpy(rsp + 3, &msg, sizeof(msg));
    rsp[3 + sizeof(msg)] = (uint8_t)((crc >> 8) & 0xff);
    rsp[4 + sizeof(msg)] = (uint8_t)(crc & 0xff);
    rsp[5 + sizeof(msg)] = PARAM_TAIL;

    ptl_send_data(rsp, 6 + sizeof(msg));
}
#endif

