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
#include "dbgserial.h"
#include "stm32f10x_cfg.h"
#include "config.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ptl]"


typedef bool (*ptl_process)(const uint8_t *data, uint8_t len, void *args);
static ptl_process ptls[] =
{
#ifdef __MASTER
    process_robot_data,
#endif
    process_param_data
};


/* serial handle */
static serial *g_serial = NULL;

#if DUMP_PROTOCOL
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
        dbg_putchar("0123456789abcdef"[data[i] >> 4]);
        dbg_putchar("0123456789abcdef"[data[i] & 0x0f]);
        dbg_putchar(' ');
    }
    dbg_putchar('\r');
    dbg_putchar('\n');
}
#endif

/**
 * @brief send protocol data
 * @param data - data to analyze
 * @param len - data length
 */
void ptl_send_data(const uint8_t *data, uint8_t len)
{
    assert_param(NULL != g_serial);
    serial_putstring(g_serial, (const char *)data, len);
#if DUMP_PROTOCOL
    dump_message(1, data, len);
#endif
}

/**
 * @brief led monitor task
 * @param pvParameters - task parameter
 */
static void vProtocol(void *pvParameters)
{
    serial *pserial = pvParameters;
    TickType_t xDelay = 10 / portTICK_PERIOD_MS;
    uint8_t recv_data[36];
    uint8_t *pdata = recv_data;
    char data = 0;
    uint8_t len = 0;
#ifdef __MASTER
    robot_wn_type_t wn_type = ROBOT_WN;
#endif
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
#if DUMP_PROTOCOL
            dump_message(0, recv_data, len);
#endif
            for (uint8_t index = 0; index < sizeof(ptls) / sizeof(ptls[0]); ++index)
            {
#ifdef __MASTER
                if (ptls[index](recv_data, len, &wn_type))
                {
                    break;
                }
#else
                if (ptls[index](recv_data, len, NULL))
                {
                    break;
                }
#endif
            }
        }
    }
}

/**
 * @brief initialize protocol
 * @return init status
 */
bool ptl_init(void)
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

