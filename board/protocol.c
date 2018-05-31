/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "protocol.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "serial.h"
#include "global.h"
#include "parameter.h"
#include "elevator.h"
#include "ledstatus.h"
#include "robot.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ledmtl]"

/* protocol head and tail */
#define PROTOCOL_HEAD    0x02
#define PROTOCOL_TAIL    0x03

#define PROTOCOL_CONVERT     0x04
#define PROTOCOL_CONVERT_2   0x06
#define PROTOCOL_CONVERT_3   0x07

#define PROTOCOL_CONVERT_ORIGIN     0x04
#define PROTOCOL_CONVERT_2_ORIGIN   0x02
#define PROTOCOL_CONVERT_3_ORIGIN   0x03

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
    }_status;
}elev_status;

/* serial handle */
static serial *g_serial = NULL;
process_cb arrive_cb = NULL;

static void process_elev_apply(const uint8_t *data, uint8_t len);
static void process_elev_release(const uint8_t *data, uint8_t len);
static void process_elev_checkin(const uint8_t *data, uint8_t len);
static void process_elev_inquire(const uint8_t *data, uint8_t len);
static void process_elev_door(const uint8_t *data, uint8_t len);
static void process_elev_arrive(const uint8_t *data, uint8_t len);

/* process handle */
typedef struct
{
    uint8_t cmd;
    void (*process)(const uint8_t *data, uint8_t len);
}cmd_handle;

cmd_handle cmd_handles[] = 
{
    {50, process_elev_apply},
    {52, process_elev_release},
    {30, process_elev_checkin},
    {32, process_elev_inquire},
    {34, process_elev_door},
    {40, process_elev_arrive},
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
static void unpacket_data(const uint8_t *data, uint8_t len)
{
    if (sum_check(data, len))
    {
        uint8_t payload[30];
        uint8_t *pdata = payload;
        data++;
        for (int i = 0; i < len - 4; ++i)
        {
            if (PROTOCOL_CONVERT == data[i])
            {
                if (PROTOCOL_CONVERT_2 == data[i + 1])
                {
                    ++i;
                    *pdata ++ = PROTOCOL_CONVERT_2_ORIGIN;
                    continue;
                }
                else if (PROTOCOL_CONVERT_3 == data[i + 1])
                {
                    ++i;
                    *pdata ++ = PROTOCOL_CONVERT_3_ORIGIN;
                    continue;
                }
                else if (PROTOCOL_CONVERT == data[i + 1])
                {
                    ++i;
                    *pdata ++ = PROTOCOL_CONVERT_ORIGIN;
                    continue;
                }
            }

            *pdata ++ = data[i];
        }

        for (int i = 0; i < sizeof(cmd_handles) / sizeof(cmd_handles[0]); ++i)
        {
            if (payload[4] == cmd_handles[i].cmd)
            {
                cmd_handles[i].process(payload, (uint8_t)(pdata - payload));
            }
        }
    }
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
    *pdata ++= PROTOCOL_HEAD;
    uint16_t sum = 0;
    for (int i = 0; i < len; ++i)
    {
        if (PROTOCOL_CONVERT_ORIGIN == data[i])
        {
            *pdata ++ = PROTOCOL_CONVERT;
            *pdata ++ = PROTOCOL_CONVERT;
        }
        else if (PROTOCOL_CONVERT_2_ORIGIN == data[i])
        {
            *pdata ++ = PROTOCOL_CONVERT;
            *pdata ++ = PROTOCOL_CONVERT_2;
        }
        else if (PROTOCOL_CONVERT_3_ORIGIN == data[i])
        {
            *pdata ++ = PROTOCOL_CONVERT;
            *pdata ++ = PROTOCOL_CONVERT_3;
        }
        else
        {
            *pdata ++ = data[i];
        }
        sum += data[i];
    }

    pdata[1] = sum % 10 + 0x30;
    sum /= 10;
    pdata[0] = sum % 10 + 0x30;
    pdata[2] = PROTOCOL_TAIL;
    pdata += 3;

    assert_param(NULL != g_serial);
    serial_putstring(g_serial, (const char *)pdata, pdata - buffer);
}


/**
 * @brief led monitor task
 * @param pvParameters - task parameter
 */
static void vProtocol(void *pvParameters)
{
    serial *pserial = pvParameters;
    TickType_t xDelay = 50 / portTICK_PERIOD_MS;
    uint8_t recv_data[36];
    uint8_t *pdata = recv_data;
    char data = 0;
    uint8_t len = 0;
    for (;;)
    {
        if (serial_getchar(pserial, &data, portMAX_DELAY))
        {
            *pdata++ = (uint8_t)data;
            while (serial_getchar(pserial, &data, xDelay))
            {
                *pdata++ = (uint8_t)data;
            }
            len = (uint8_t)(pdata - recv_data);
            if (len > 4)
            {
                if ((PROTOCOL_HEAD == recv_data[0]) && 
                    (PROTOCOL_TAIL == pdata[-1]))
                {
                    /* receive protocol data */
                    unpacket_data(recv_data, len);
                }
            }
            pdata = recv_data;
        }
    }
}

/**
 * @brief process elevator apply message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_apply(const uint8_t *data, uint8_t len)
{
    if ((data[0] == param_get_id_ctl()) &&
        (data[2] == param_get_id_elev()))
    {
        uint8_t payload[8];
        payload[0] = param_get_id_ctl();
        payload[1] = param_get_id_elev();
        payload[2] = data[1];
        payload[3] = 51;
        payload[4] = elev_floor();
        payload[5] = data[4];
        
        elev_status status;
        status._status.dir = elev_state_run();
        status._status.led = (is_ledon(data[5]) ? 0x02 : 0x01);
        status._status.door = 0x01;
        status._status.reserve = 0x00;
        status._status.state = elev_state_work();
        payload[6] = status.status;
        
        send_data(payload, 7);
        elevator_set_state_work(work_robot);
    }
}

/**
 * @brief process elevator release message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_release(const uint8_t *data, uint8_t len)
{
    if ((data[0] == param_get_id_ctl()) &&
        (data[2] == param_get_id_elev()))
    {
        uint8_t payload[7];
        payload[0] = param_get_id_ctl();
        payload[1] = param_get_id_elev();
        payload[2] = data[1];
        payload[3] = 53;
        payload[4] = data[4];
        payload[5] = 0x00;
        
        send_data(payload, 6);
        elevator_set_state_work(work_idle);
        robot_checkin_reset(data[1]);
    }
}

/**
 * @brief process elevator checkin message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_checkin(const uint8_t *data, uint8_t len)
{
    if ((data[0] == param_get_id_ctl()) &&
        (data[2] == param_get_id_elev()))
    {
        uint8_t payload[7];
        payload[0] = param_get_id_ctl();
        payload[1] = param_get_id_elev();
        payload[2] = data[1];
        payload[3] = 31;
        payload[4] = data[4];
        payload[5] = data[5];
        
        send_data(payload, 6);
        robot_checkin_set(data[1], data[4]);
    }
}

/**
 * @brief process elevator inquire message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_inquire(const uint8_t *data, uint8_t len)
{
    if ((data[0] == param_get_id_ctl()) &&
        (data[2] == param_get_id_elev()))
    {
        uint8_t payload[8];
        payload[0] = param_get_id_ctl();
        payload[1] = param_get_id_elev();
        payload[2] = data[1];
        payload[3] = 33;
        payload[4] = elev_floor();
        payload[5] = robot_checkin_get(data[1]);
        
        elev_status status;
        status._status.dir = elev_state_run();
        status._status.led = (is_ledon(data[5]) ? 0x02 : 0x01);
        status._status.door = 0x01;
        status._status.reserve = 0x00;
        status._status.state = elev_state_work();
        payload[6] = status.status;
        
        send_data(payload, 7);
    }
}

/**
 * @brief process elevator door message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_door(const uint8_t *data, uint8_t len)
{
    if ((data[0] == param_get_id_ctl()) &&
        (data[2] == param_get_id_elev()))
    {
        uint8_t payload[6];
        payload[0] = param_get_id_ctl();
        payload[1] = param_get_id_elev();
        payload[2] = data[1];
        payload[3] = 35;
        if (0x00 == data[4])
        {
            /* open */
            payload[4] = 0x00;
        }
        else 
        {
            /* release */
            payload[4] = 0x01;
        }
        
        send_data(payload, 5);
        
        if (0x00 == data[4])
        {
            /* open */
            elev_hold_open(TRUE);
        }
        else 
        {
            /* release */
            payload[4] = 0x01;
            elev_hold_open(FALSE);
        }
    }
}

/**
 * @brief process elevator door message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_arrive(const uint8_t *data, uint8_t len)
{
    if ((data[0] == param_get_id_ctl()) &&
        (data[2] == param_get_id_elev()))
    {
        if (NULL != arrive_cb)
        {
            arrive_cb(data, len);
        }
    }
}


/**
 * @brief process elevator door message
 * @param data - data to process
 * @param len - data length
 */
void notify_arrive(uint8_t floor)
{
    uint8_t payload[7];
    payload[0] = param_get_id_ctl();
    payload[1] = param_get_id_elev();
    payload[2] = robot_id_get(floor);
    payload[3] = 39;
    payload[4] = elev_floor();
    elev_status status;
    status._status.dir = elev_state_run();
    status._status.led = (is_ledon(payload[2]) ? 0x02 : 0x01);
    status._status.door = 0x01;
    status._status.reserve = 0x00;
    status._status.state = elev_state_work();
    payload[5] = status.status;
    
    send_data(payload, 6);
}

/**
 * @brief register arrive message callback
 * @param cb - callback
 */
void register_arrive_cb(process_cb cb)
{
    arrive_cb = cb;
}

/**
 * @brief initialize protocol
 * @return init status
 */
bool protocol_init(void)
{
    g_serial = serial_request(COM1);
    if (NULL == g_serial)
    {
        TRACE("initialize failed, can't open serial \'COM1\'\r\n");
        return FALSE;
    }
    serial_open(g_serial);
    xTaskCreate(vProtocol, "protocol", PROTOCOL_STACK_SIZE, g_serial,
                    PROTOCOL_PRIORITY, NULL);

    return TRUE;
}


