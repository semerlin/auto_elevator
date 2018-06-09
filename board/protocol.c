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
#include "led_status.h"
#include "robot.h"
#include "floormap.h"
#include "elevator.h"
#include "crc.h"
#include "keymap.h"
#include "keyctl.h"
#include "led_monitor.h"
#include "switch_monitor.h"
#include "dbgserial.h"
#include "stm32f10x_cfg.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ptl]"

/* protocol head and tail */
#define ROBOT_HEAD    0x02
#define ROBOT_TAIL    0x03

#define PARAM_HEAD    0x55
#define PARAM_TAIL    0xaa

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
static void process_elev_door_open(const uint8_t *data, uint8_t len);
static void process_elev_door_close(const uint8_t *data, uint8_t len);
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
    {34, process_elev_door_open},
    {36, process_elev_door_close},
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
static void unpacket_robot_data(const uint8_t *data, uint8_t len)
{
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
                    cmd_handles[i].process(payload, (uint8_t)(pdata - payload));
                }
            }
        }
    }
}

/**
 * @brief replay to init
 * @param flag - init status
 */
static void init_reply(bool flag)
{
    uint8_t rsp[7];
    uint8_t status = (flag ? 0x00 : 0x01);
    uint16_t crc = crc16(&status, 1);
    rsp[0] = PARAM_HEAD;
    rsp[1] = 6;
    rsp[2] = status;
    rsp[3] = (uint8_t)((crc >> 8) & 0xff);
    rsp[4] = (uint8_t)(crc & 0xff);
    rsp[5] = PARAM_TAIL;
    
    serial_putstring(g_serial, (const char *)rsp, 6);
}

/**
 * @brief analyze protocol data
 * @param data - data to analyze
 * @param len - data length
 */
static void unpacket_param_data(const uint8_t *data, uint8_t len)
{
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
            if(param_update_all(data + 2))
            {
                init_reply(TRUE);
                TRACE("rebooting...\r\n");
                SCB_SystemReset();
            }
            else
            {
                init_reply(FALSE);
                TRACE("initialize parameter failed!\r\n");
            }
        }
    }
}

/* characteristic map for dump message */
static uint8_t char_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', 
'9', 'a', 'b', 'c', 'd', 'e', 'f'};

/**
 * @brief dump message
 * @param data - message to dump
 * @param len - data length
 */
static void dump_message(uint8_t dir, const uint8_t *data, uint8_t len)
{
    if (dir)
    {
        TRACE("send data: ");
    }
    else
    {
        TRACE("recv data: ");
    }
    for (int i = 0; i < len; ++i)
    {
        dbg_putchar(char_map[data[i] >> 4]);
        dbg_putchar(char_map[data[i] & 0x0f]);
        dbg_putchar(' ');
    }
    dbg_putchar('\r');
    dbg_putchar('\n');
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
    *pdata ++= ROBOT_HEAD;
    uint16_t sum = 0;
    for (int i = 0; i < len; ++i)
    {
        if (CONVERT_ORIGIN == data[i])
        {
            *pdata ++ = CONVERT;
            *pdata ++ = CONVERT;
        }
        else if (CONVERT_2_ORIGIN == data[i])
        {
            *pdata ++ = CONVERT;
            *pdata ++ = CONVERT_2;
        }
        else if (CONVERT_3_ORIGIN == data[i])
        {
            *pdata ++ = CONVERT;
            *pdata ++ = CONVERT_3;
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
    pdata[2] = ROBOT_TAIL;
    pdata += 3;

    assert_param(NULL != g_serial);
    uint8_t datalen = (uint8_t)(pdata - buffer);
#ifdef __ENABLE_TRACE
    dump_message(1, buffer, datalen);
#endif
    serial_putstring(g_serial, (const char *)buffer, pdata - buffer);
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
        len = 0;
        pdata = recv_data;
        if (serial_getchar(pserial, &data, portMAX_DELAY))
        {
            *pdata++ = (uint8_t)data;
            len ++;
            while (serial_getchar(pserial, &data, xDelay))
            {
                *pdata++ = (uint8_t)data;
                len ++;
                if (len > 34)
                {
                    /* TODO: add process */
                    break;
                }
            }
#ifdef __ENABLE_TRACE
            dump_message(0, recv_data, len);
#endif
            if (is_param_setted())
            {
                /* process robot data */
                if (len > 4)
                {
                    if ((ROBOT_HEAD == recv_data[0]))
                    {
                        /* receive protocol data */
                        unpacket_robot_data(recv_data, len);
                    }
                    else if (PARAM_HEAD == recv_data[0])
                    {
                        /* process parameter data */
                        unpacket_param_data(recv_data, len);
                    }
                }
            }
            else
            {
                if (PARAM_HEAD == recv_data[0])
                {
                    /* process parameter data */
                    unpacket_param_data(recv_data, len);
                }
            }
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
        
        elev_hold_open(FALSE);
        robot_checkin_reset(data[1]);
        elevator_set_state_work(work_idle);
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
        if(work_robot == elev_state_work())
        {
            uint8_t payload[7];
            payload[0] = param_get_id_ctl();
            payload[1] = param_get_id_elev();
            payload[2] = data[1];
            payload[3] = 31;
            payload[4] = data[4];
            payload[5] = data[5];
            
            send_data(payload, 6);
            char dis_floor = floormap_phy_to_dis(data[4]);
            robot_checkin_set(data[1], data[4]);
            elev_go(dis_floor);
        }
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
        payload[4] = floormap_dis_to_phy(elev_floor());
        payload[5] = robot_checkin_get(data[1]);
        
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
}

/**
 * @brief process elevator door open message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_door_open(const uint8_t *data, uint8_t len)
{
    if ((data[0] == param_get_id_ctl()) &&
        (data[2] == param_get_id_elev()))
    {
        if(work_robot == elev_state_work())
        {
            uint8_t payload[6];
            payload[0] = param_get_id_ctl();
            payload[1] = param_get_id_elev();
            payload[2] = data[1];
            payload[3] = 35;
         
            send_data(payload, 4);
            
            /* open */
            elev_hold_open(TRUE);
        }
    }
}


/**
 * @brief process elevator door close message
 * @param data - data to process
 * @param len - data length
 */
static void process_elev_door_close(const uint8_t *data, uint8_t len)
{
    if ((data[0] == param_get_id_ctl()) &&
        (data[2] == param_get_id_elev()))
    {
        if(work_robot == elev_state_work())
        {
            uint8_t payload[6];
            payload[0] = param_get_id_ctl();
            payload[1] = param_get_id_elev();
            payload[2] = data[1];
            payload[3] = 37;
            
            send_data(payload, 4);
            
            /* release */
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
void notify_arrive(char floor)
{
    uint8_t payload[7];
    payload[0] = param_get_id_ctl();
    payload[1] = param_get_id_elev();
    payload[2] = robot_id_get(floormap_dis_to_phy(floor));
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
    TRACE("initizlize protocol...\r\n");
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


