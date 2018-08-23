/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifdef __MASTER
#include "protocol.h"
#include "protocol_robot.h"
#include "trace.h"
#include "global.h"
#include "parameter.h"
#include "led_status.h"
#include "robot.h"
#include "floormap.h"
#include "elevator.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ptl_robot]"

extern parameters_t board_parameter;
/* protocol head and tail */
#define ROBOT_HEAD    0x02
#define ROBOT_TAIL    0x03

#define CONVERT     0x04
#define CONVERT_2   0x06
#define CONVERT_3   0x07

#define CONVERT_ORIGIN     0x04
#define CONVERT_2_ORIGIN   0x02
#define CONVERT_3_ORIGIN   0x03

#define DEFAULT_FLOOR   0xf7
#define LED_ON          0x02
#define LED_OFF         0x01
#define DOOR_ON         0x01
#define DOOR_OFF        0x00

static process_robot_cb arrive_cb = NULL;

typedef struct
{
    uint8_t ctl_id;
    uint8_t robot_id;
    uint8_t elev_id;
    uint8_t cmd;
} recv_head;

typedef union
{
    uint8_t status;
    struct
    {
        uint8_t dir : 2;
        uint8_t led : 2;
        uint8_t door : 1;
        uint8_t reserve : 1;
        uint8_t state : 2;
    } _status;
} elev_status;

static void process_elev_apply(const uint8_t *data, uint8_t len);
static void process_elev_release(const uint8_t *data, uint8_t len);
static void process_elev_checkin(const uint8_t *data, uint8_t len);
static void process_elev_inquire(const uint8_t *data, uint8_t len);
static void process_elev_door_open(const uint8_t *data, uint8_t len);
static void process_elev_door_close(const uint8_t *data, uint8_t len);
static void process_elev_arrive(const uint8_t *data, uint8_t len);
static void notify_busy(uint8_t id);

/* process handle */
typedef struct
{
    uint8_t cmd;
    void (*process)(const uint8_t *data, uint8_t len);
} cmd_handle;

/* protocol command */
#define CMD_APPLY         50
#define CMD_RELEASE       52
#define CMD_CHECKIN       30
#define CMD_INQUIRE       32
#define CMD_DOOR_OPEN     34
#define CMD_DOOR_CLOSE    36
#define CMD_ARRIVE        40

static cmd_handle cmd_handles[] =
{
    {CMD_APPLY, process_elev_apply},
    {CMD_RELEASE, process_elev_release},
    {CMD_CHECKIN, process_elev_checkin},
    {CMD_INQUIRE, process_elev_inquire},
    {CMD_DOOR_OPEN, process_elev_door_open},
    {CMD_DOOR_CLOSE, process_elev_door_close},
    {CMD_ARRIVE, process_elev_arrive},
};

/**
 * @brief data check
 * @param data - data to check
 * @param len - data length
 */
static bool sum_check(const uint8_t *data, uint8_t len)
{
    uint16_t sum = 0;
    uint8_t recv_check[2];
    uint8_t calc_check[2];
    for (int i = 0; i < len - 4; ++i)
    {
        sum += data[i + 1];
    }
    /* get received check */
    recv_check[0] = data[len - 3];
    recv_check[1] = data[len - 2];
    /* calculate check */
    calc_check[1] = sum % 10 + 0x30;
    sum /= 10;
    calc_check[0] = sum % 10 + 0x30;
    return ((recv_check[0] == calc_check[0]) &&
            (recv_check[1] == calc_check[1]));
}

/**
 * @brief analyze protocol data
 * @param data - data to analyze
 * @param len - data length
 */
bool process_robot_data(const uint8_t *data, uint8_t len)
{
    if ((ROBOT_HEAD != data[0]))
    {
        return FALSE;
    }

    /* check tail */
    if (ROBOT_TAIL == data[len - 1])
    {
        if (sum_check(data, len))
        {
            uint8_t payload[30];
            uint8_t *pdata = payload;
            data++;
            for (int i = 0; i < len - 4; ++i)
            {
                if (CONVERT == data[i])
                {
                    if (CONVERT_2 == data[i + 1])
                    {
                        ++i;
                        *pdata ++ = CONVERT_2_ORIGIN;
                        continue;
                    }
                    else if (CONVERT_3 == data[i + 1])
                    {
                        ++i;
                        *pdata ++ = CONVERT_3_ORIGIN;
                        continue;
                    }
                    else if (CONVERT == data[i + 1])
                    {
                        ++i;
                        *pdata ++ = CONVERT_ORIGIN;
                        continue;
                    }
                }

                *pdata ++ = data[i];
            }

            for (int i = 0; i < sizeof(cmd_handles) / sizeof(cmd_handles[0]); ++i)
            {
                if (payload[3] == cmd_handles[i].cmd)
                {
                    recv_head head;
                    head.ctl_id = payload[0];
                    head.robot_id = payload[1];
                    head.elev_id = payload[2];
                    head.cmd = payload[3];
                    /* check control and elevator address */
                    if ((head.ctl_id == board_parameter.id_ctl) &&
                        (head.elev_id == board_parameter.id_elev))
                    {
                        if (work_robot == elev_state_work())
                        {
                            /* check robot address */
                            if (head.robot_id == robot_id_get())
                            {
                                robot_monitor_reset();
                                if (CMD_APPLY != head.cmd)
                                {
                                    cmd_handles[i].process(payload, (uint8_t)(pdata - payload));
                                }
                                else
                                {
                                    /* notify busy */
                                    notify_busy(head.robot_id);
                                }
                            }
                            else
                            {
                                /* notify busy */
                                notify_busy(head.robot_id);
                            }
                        }
                        else
                        {
                            /* only process apply command */
                            if (CMD_APPLY == head.cmd)
                            {
                                cmd_handles[i].process(payload, (uint8_t)(pdata - payload));
                            }
                            else
                            {
                                notify_busy(head.robot_id);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    return TRUE;
}

/**
 * @brief send protocol data
 * @param data - data to analyze
 * @param len - data length
 */
static void send_data(const uint8_t *data, uint8_t len)
{
    uint8_t buffer[36];
    uint8_t *pdata = buffer;
    *pdata ++ = ROBOT_HEAD;
    uint16_t sum = 0;
    for (int i = 0; i < len; ++i)
    {
        if (CONVERT_ORIGIN == data[i])
        {
            *pdata ++ = CONVERT;
            *pdata ++ = CONVERT;
            sum += CONVERT;
            sum += CONVERT;
        }
        else if (CONVERT_2_ORIGIN == data[i])
        {
            *pdata ++ = CONVERT;
            *pdata ++ = CONVERT_2;
            sum += CONVERT;
            sum += CONVERT_2;
        }
        else if (CONVERT_3_ORIGIN == data[i])
        {
            *pdata ++ = CONVERT;
            *pdata ++ = CONVERT_3;
            sum += CONVERT;
            sum += CONVERT_3;
        }
        else
        {
            *pdata ++ = data[i];
            sum += data[i];
        }
    }

    pdata[1] = sum % 10 + 0x30;
    sum /= 10;
    pdata[0] = sum % 10 + 0x30;
    pdata[2] = ROBOT_TAIL;
    pdata += 3;

    ptl_send_data(buffer, pdata - buffer);
}

/**
 * @brief process elevator apply message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_apply(const uint8_t *data, uint8_t len)
{
    uint8_t payload[8];
    payload[0] = board_parameter.id_ctl;
    payload[1] = board_parameter.id_elev;
    payload[2] = data[1];
    payload[3] = 51;
    payload[4] = floormap_dis_to_phy(elev_floor());
    payload[5] = data[4];

    elev_status status;
    status._status.dir = elev_state_run();
    if (DEFAULT_FLOOR == data[5])
    {
        status._status.led = LED_OFF;
    }
    else
    {
        char dis_floor = floormap_phy_to_dis(data[5]);
        status._status.led = (is_led_on(dis_floor) ? LED_ON : LED_OFF);
    }
    status._status.door = DOOR_ON;
    status._status.reserve = 0x00;
    status._status.state = elev_state_work();
    payload[6] = status.status;

    send_data(payload, 7);
    robot_id_set(data[1]);
    elevator_set_state_work(work_robot);
    robot_monitor_start();
}

/**
 * @brief process elevator release message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_release(const uint8_t *data, uint8_t len)
{
    uint8_t payload[7];
    payload[0] = board_parameter.id_ctl;
    payload[1] = board_parameter.id_elev;
    payload[2] = data[1];
    payload[3] = 53;
    payload[4] = data[4];
    payload[5] = 0x00;
    ptl_send_data(payload, 6);

    elev_hold_open(FALSE);
    robot_id_reset();
    robot_checkin_reset();
    elevator_set_state_work(work_idle);
    robot_monitor_stop();
}

/**
 * @brief process elevator checkin message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_checkin(const uint8_t *data, uint8_t len)
{
    uint8_t payload[7];
    payload[0] = board_parameter.id_ctl;
    payload[1] = board_parameter.id_elev;
    payload[2] = data[1];
    payload[3] = 31;
    payload[4] = data[4];
    payload[5] = data[5];

    send_data(payload, 6);
    char dis_floor = floormap_phy_to_dis(data[4]);
    robot_checkin_set(data[4]);
    /* goto specified floor */
    elev_go(dis_floor);
    if (dis_floor == elev_floor())
    {
        /* already arrive */
        elev_arrived(dis_floor);
    }
}

/**
 * @brief process elevator inquire message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_inquire(const uint8_t *data, uint8_t len)
{
    uint8_t payload[8];
    payload[0] = board_parameter.id_ctl;
    payload[1] = board_parameter.id_elev;
    payload[2] = data[1];
    payload[3] = 33;
    payload[4] = floormap_dis_to_phy(elev_floor());
    payload[5] = robot_checkin_get();

    elev_status status;
    status._status.dir = elev_state_run();
    if (DEFAULT_FLOOR == payload[5])
    {
        status._status.led = LED_OFF;
    }
    else
    {
        char dis_floor = floormap_phy_to_dis(payload[5]);
        status._status.led = (is_led_on(dis_floor) ? LED_ON : LED_OFF);
    }
    status._status.door = DOOR_ON;
    status._status.reserve = 0x00;
    status._status.state = elev_state_work();
    payload[6] = status.status;

    send_data(payload, 7);
}

/**
 * @brief process elevator door open message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_door_open(const uint8_t *data, uint8_t len)
{
    uint8_t payload[6];
    payload[0] = board_parameter.id_ctl;
    payload[1] = board_parameter.id_elev;
    payload[2] = data[1];
    payload[3] = 35;

    send_data(payload, 4);

    /* open */
    elev_hold_open(TRUE);
}


/**
 * @brief process elevator door close message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_door_close(const uint8_t *data, uint8_t len)
{
    uint8_t payload[6];
    payload[0] = board_parameter.id_ctl;
    payload[1] = board_parameter.id_elev;
    payload[2] = data[1];
    payload[3] = 37;

    send_data(payload, 4);

    /* release */
    elev_hold_open(FALSE);
}

/**
 * @brief process elevator door message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_arrive(const uint8_t *data, uint8_t len)
{
    if (NULL != arrive_cb)
    {
        arrive_cb(data, len);
    }
}


/**
 * @brief process elevator arrive
 * @param floor - arrive floor
 */
void notify_arrive(char floor)
{
    uint8_t payload[7];
    payload[0] = board_parameter.id_ctl;
    payload[1] = board_parameter.id_elev;
    payload[2] = robot_id_get();
    payload[3] = 39;
    payload[4] = floormap_dis_to_phy(elev_floor());
    elev_status status;
    status._status.dir = elev_state_run();
    status._status.led = (is_led_on(floor) ? LED_ON : LED_OFF);
    status._status.door = DOOR_ON;
    status._status.reserve = 0x00;
    status._status.state = elev_state_work();
    payload[5] = status.status;

    send_data(payload, 6);
}

/**
 * @brief process elevator busy message
 */
static void notify_busy(uint8_t id)
{
    uint8_t payload[8];
    payload[0] = board_parameter.id_ctl;
    payload[1] = board_parameter.id_elev;
    payload[2] = id;
    payload[3] = 55;
    payload[4] = floormap_dis_to_phy(elev_floor());
    payload[5] = robot_id_get();

    elev_status status;
    status._status.dir = elev_state_run();
    if (DEFAULT_FLOOR == robot_checkin_get())
    {
        status._status.led = LED_OFF;
    }
    else
    {
        char dis_floor = floormap_phy_to_dis(robot_checkin_get());
        status._status.led = (is_led_on(dis_floor) ? LED_ON : LED_OFF);
    }
    status._status.door = DOOR_ON;
    status._status.reserve = 0x00;
    status._status.state = elev_state_work();
    payload[6] = status.status;

    send_data(payload, 7);
}

/**
 * @brief register arrive message callback
 * @param cb - callback
 */
void register_arrive_cb(process_robot_cb cb)
{
    arrive_cb = cb;
}
#endif

