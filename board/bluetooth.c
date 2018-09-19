/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifdef __MASTER
#include <string.h>
#include <stdio.h>
#include "bluetooth.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "serial.h"
#include "global.h"
#include "dbgserial.h"
#include "stm32f10x_cfg.h"
#include "config.h"
#include "protocol_robot.h"
#include "parameter.h"


#undef __TRACE_MODULE
#define __TRACE_MODULE  "[bt]"

extern parameters_t board_parameter;
/* serial handle */
static serial *g_serial = NULL;

#if DUMP_BT
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
void bt_send_data(const uint8_t *data, uint8_t len)
{
    assert_param(NULL != g_serial);
    serial_putstring(g_serial, (const char *)data, len);
#if DUMP_BT
    dump_message(1, data, len);
#endif
}

/**
 * @brief set bluetooth name
 * @param[in] name: bluetooth name
 * @return set status
 */
bool bt_set_name(const char *name)
{
    char bt_name[BT_NAME_MAX_LEN + 10];
    memset(bt_name, 0, BT_NAME_MAX_LEN + 10);
    sprintf(bt_name, "AT+NAME=%s", name);
    bt_send_data((const uint8_t *)bt_name, strlen(bt_name) - 1);
    return TRUE;
}

/**
 * @brief led monitor task
 * @param pvParameters - task parameter
 */
static void vBluetooth(void *pvParameters)
{
    serial *pserial = pvParameters;
    TickType_t xDelay = 10 / portTICK_PERIOD_MS;
    uint8_t recv_data[36];
    uint8_t *pdata = recv_data;
    char data = 0;
    uint8_t len = 0;
    robot_wn_type_t wn_type = ROBOT_BT;
    /** set bluetooth name */
    vTaskDelay(500 / portTICK_PERIOD_MS);
    bt_set_name((const char *)board_parameter.bt_name);
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
            process_robot_data(recv_data, len, &wn_type);
        }
    }
}

/**
 * @brief initialize protocol
 * @return init status
 */
bool bt_init(void)
{
    TRACE("initizlize bluetooth...\r\n");
    g_serial = serial_request(COM5);
    if (NULL == g_serial)
    {
        TRACE("initialize failed, can't open serial \'COM5\'\r\n");
        return FALSE;
    }
    serial_open(g_serial);
    xTaskCreate(vBluetooth, "bluetooth", BLUETOOTH_STACK_SIZE, g_serial,
                BLUETOOTH_PRIORITY, NULL);

    return TRUE;
}

#endif
