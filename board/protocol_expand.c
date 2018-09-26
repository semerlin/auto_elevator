/**
 * This file is part of the auto-elevator project.
 *
 * Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "protocol_expand.h"
#include "expand.h"
#include "global.h"
#include "trace.h"
#include "parameter.h"
#include "elevator.h"
#include "boardmap.h"
#include "floormap.h"
#include "led_monitor.h"


#undef __TRACE_MODULE
#define __TRACE_MODULE  "[EXPAND_PTL]"

typedef enum
{
    SUCCESS,
    ID_EXISTS,
    FLOOR_EXISTS,
    REGISTER_FAIL,
} expand_status_t;

extern parameters_t board_parameter;

#ifdef __EXPAND
static register_cb_t register_cb_func = NULL;
#endif

static void process_board_register(const uint8_t *data, uint8_t len);
#ifdef __MASTER
static void process_elev_led(const uint8_t *data, uint8_t len);
#endif
#ifdef __EXPAND
static void process_elev_go(const uint8_t *data, uint8_t len);
static void process_reboot(const uint8_t *data, uint8_t len);
#endif

#pragma pack(1)
typedef struct
{
    uint8_t id_board;
    char start_floor;
} msg_board_register_t;

typedef struct
{
    uint8_t id_board;
    uint16_t led_status;
} msg_led_status_t;

typedef struct
{
    uint8_t id_board;
    char floor;
} msg_elev_go_t;

typedef struct
{
    uint8_t status;
    uint8_t id_board;
} msg_board_register_status_t;

typedef struct
{
    uint8_t id_board;
} msg_reboot_t;

#pragma pack()


/* process handle */
typedef struct
{
    uint8_t cmd;
    void (*process)(const uint8_t *data, uint8_t len);
} cmd_handle;

/* protocol command */
#define CMD_BOARD_REGISTER     0x01
#define CMD_ELEV_LED           0x02
#define CMD_ELEV_GO            0x03
#define CMD_REBOOT             0x04

static cmd_handle cmd_handles[] =
{
    {CMD_BOARD_REGISTER, process_board_register},
#ifdef __MASTER
    {CMD_ELEV_LED, process_elev_led},
#endif
#ifdef __EXPAND
    {CMD_ELEV_GO, process_elev_go},
    {CMD_REBOOT, process_reboot},
#endif
};

bool process_expand_data(const uint8_t *data, uint8_t len)
{
    for (int i = 0; i < sizeof(cmd_handles) / sizeof(cmd_handles[0]); ++i)
    {
        if (data[0] == cmd_handles[i].cmd)
        {
            cmd_handles[i].process(data + 1, len - 1);
            break;
        }
    }
    return TRUE;
}

/**
 * @brief send expand protocol data
 * @param[in] cmd: expand command
 * @param[in] data: command parameter
 * @param[in] len: command parameter length
 */
static void expand_ptl_send(uint8_t cmd, uint8_t *data, uint8_t len)
{
    uint8_t rsp[20];
    rsp[0] = cmd;
    for (uint8_t i = 0; i < len; ++i)
    {
        rsp[1 + i] = data[i];
    }
    expand_send_data(rsp, len + 1);
}

#ifdef __MASTER
/**
 * @brief reply to expand protocol data
 * @param[in] cmd: expand command
 * @param[in] status: command execute status
 * @param[in] data: command parameter
 * @param[in] len: command parameter length
 */
static void expand_ptl_reply(uint8_t cmd, uint8_t status, uint8_t *data, uint8_t len)
{
    uint8_t rsp[20];
    rsp[0] = cmd;
    rsp[1] = status;
    for (uint8_t i = 0; i < len; ++i)
    {
        rsp[2 + i] = data[i];
    }
    expand_send_data(rsp, len + 2);
}

/**
 * @brief process board register information
 * @param[in] data: register data
 * @param[in] len: register data len
 */
static void process_board_register(const uint8_t *data, uint8_t len)
{
    msg_board_register_t *pmsg = (msg_board_register_t *)data;
    TRACE("expand want to register:%d-%d\r\n", pmsg->id_board, pmsg->start_floor);
    /** check start */
    if (floormap_contains_floor(pmsg->start_floor))
    {
        expand_ptl_reply(CMD_BOARD_REGISTER, FLOOR_EXISTS, &pmsg->id_board, 1);
        return ;
    }

    char end_floor = pmsg->start_floor + MAX_FLOOR_NUM;
    if ((pmsg->start_floor < 0) && (end_floor >= 0))
    {
        end_floor += 1;
    }

    /** check end, so range checked */
    if (floormap_contains_floor(end_floor))
    {
        expand_ptl_reply(CMD_BOARD_REGISTER, FLOOR_EXISTS, &pmsg->id_board, 1);
        return ;
    }

    if (boardmap_is_board_id_exists(pmsg->id_board))
    {
        expand_ptl_reply(CMD_BOARD_REGISTER, ID_EXISTS, &pmsg->id_board, 1);
        return ;
    }

    /** add board */
    if (!boardmap_add(pmsg->id_board, EXPAND_START_KEY, pmsg->start_floor,
                      MAX_EXPAND_FLOOR_NUM, 0))
    {
        /** no place for store */
        expand_ptl_reply(CMD_BOARD_REGISTER, REGISTER_FAIL, &pmsg->id_board, 1);
    }
    else
    {
        floormap_update();
        expand_ptl_reply(CMD_BOARD_REGISTER, SUCCESS, &pmsg->id_board, 1);
    }
}

/**
 * @brief process board led status
 * @param[in] data: led status data
 * @param[in] len: led status data length
 */
static void process_elev_led(const uint8_t *data, uint8_t len)
{
    msg_led_status_t *pmsg = (msg_led_status_t *)data;
    if (boardmap_is_board_id_exists(pmsg->id_board))
    {
        TRACE("led status:%d-0x%04x\r\n", pmsg->id_board, pmsg->led_status);
        uint16_t prev_led_status = boardmap_get_led_status(pmsg->id_board);
        if (prev_led_status != pmsg->led_status)
        {
            led_monitor_process(pmsg->id_board, prev_led_status, pmsg->led_status);
            boardmap_update_led_status(pmsg->id_board, pmsg->led_status);
        }
    }
}

/**
 * @brief notify expand board goto specified floor
 * @param[in] id_board: expand board id
 * @param[in] floor: floot to go
 */
void expand_elev_go(uint8_t id_board, uint8_t floor)
{
    uint8_t data[2] = {id_board, floor};
    expand_ptl_send(CMD_ELEV_GO, data, 2);
}

/**
 * @brief notify expand board to reboot
 * @param[in] id_board: expand board id, 0xff means all board
 */
void expand_reboot(uint8_t id_board)
{
    expand_ptl_send(CMD_REBOOT, &id_board, 1);
}
#endif

#ifdef __EXPAND
/**
 * @brief process board register information
 * @param[in] data: register data
 * @param[in] len: register data len
 */
static void process_board_register(const uint8_t *data, uint8_t len)
{
    msg_board_register_status_t *pmsg = (msg_board_register_status_t *)data;
    if (pmsg->id_board == board_parameter.id_board)
    {
        TRACE("board register status(%d)\r\n", pmsg->status);
        if (NULL != register_cb_func)
        {
            register_cb_func(&pmsg->status, 1);
        }
    }
}

/**
 * @brief process elevator goto floor message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_go(const uint8_t *data, uint8_t len)
{
    if (is_expand_board_registered())
    {
        msg_elev_go_t *pmsg = (msg_elev_go_t *)data;
        if (pmsg->id_board == board_parameter.id_board)
        {
            elev_go(pmsg->floor);
        }
    }
}

/**
 * @brief process board reboot
 * @param data - data to process
 * @param len - data length
 */
static void process_reboot(const uint8_t *data, uint8_t len)
{
    msg_reboot_t *pmsg = (msg_reboot_t *)data;
    if ((0xff == pmsg->id_board) ||
        (pmsg->id_board == board_parameter.id_board))
    {
        SCB_SystemReset();
    }
}


/**
 * @brief register board to master
 * @param[in] id_board: expand board id
 * @param[in] start_floor: expand board start floor
 */
void register_board(uint8_t id_board, uint8_t start_floor)
{
    msg_board_register_t msg;
    msg.id_board = id_board;
    msg.start_floor = start_floor;

    expand_ptl_send(CMD_BOARD_REGISTER, (uint8_t *)&msg, sizeof(msg));
    TRACE("register board: %d, %d\r\n", id_board, start_floor);
}

/**
 * @brief notify master board led status
 * @param[in] led_status: expand board led status
 */
void notify_led_status(uint8_t id_board, uint16_t led_status)
{
    msg_led_status_t msg;
    msg.id_board = id_board;
    msg.led_status = led_status;

    expand_ptl_send(CMD_ELEV_LED, (uint8_t *)&msg, sizeof(msg));
    TRACE("send led status: 0x%x\r\n", led_status);
}

/**
 * @brief set register callback function
 * @param[in] register_cb: register callback function
 */
void set_register_cb(register_cb_t register_cb)
{
    register_cb_func = register_cb;
}

#endif
