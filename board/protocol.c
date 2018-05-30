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

/* serial handle */
static serial *g_serial = NULL;

static void process_apply_elev(const uint8_t *data, uint8_t len);

/* process handle */
typedef struct
{
    uint8_t cmd;
    void (*process)(const uint8_t *data, uint8_t len);
}cmd_handle;

cmd_handle cmd_handles[] = 
{
    {50, process_apply_elev},
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
static void process_apply_elev(const uint8_t *data, uint8_t len)
{
    if ((data[0] == param_get_id_ctl()) &&
        (data[2] == param_get_id_elev()))
    {
        uint8_t robot_addr = data[1];
    }
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


