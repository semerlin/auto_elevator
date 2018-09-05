/**
* This file is part of the vendoring machine project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#ifdef __MASTER
#include <string.h>
#include "altimeter.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "serial.h"
#include "global.h"
#include "dbgserial.h"
#include "stm32f10x_cfg.h"
#include "elevator.h"
#include "parameter.h"
#include "altimeter_calc.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[altimeter]"

#define DUMP_MESSAGE 0

typedef struct
{
    uint32_t range;
    uint8_t id_tag;
    uint8_t id_base_station;
} tof_t;

#pragma pack(1)
typedef struct
{
    uint8_t range[8];
    uint8_t space;
} msg_range_t;

typedef struct
{
    uint8_t type[2];
    uint8_t space1;
    uint8_t mask[2];
    uint8_t space2;
    msg_range_t range[4];
    uint8_t nranges[4];
    uint8_t space3;
    uint8_t seq[2];
    uint8_t space4;
    uint8_t debug[8];
    uint8_t space5;
    uint8_t flag;
    uint8_t id_tag;
    uint8_t colon;
    uint8_t id_base_station;
} msg_tof_t;
#pragma pack()

static tof_t tof;

extern parameters_t board_parameter;

/* serial handle */
static serial *g_serial = NULL;

static uint32_t bin2hex(const uint8_t *data, uint8_t len)
{
    uint32_t hex_data = 0;
    if (len > 8)
    {
        len = 8;
    }

    uint32_t temp_hex = 0;
    for (uint8_t i = 0; i < len; ++i)
    {
        if ((data[i] >= '0') && (data[i] <= '9'))
        {
            temp_hex = data[i] - '0';
        }
        else if ((data[i] >= 'a') && (data[i] <= 'f'))
        {
            temp_hex = data[i] - 'a' + 0x0a;
        }
        else if ((data[i] >= 'A') && (data[i] <= 'F'))
        {
            temp_hex = data[i] - 'A' + 0x0a;
        }
        else
        {
            TRACE("invalid bin\r\n");
            return 0;
        }
        hex_data <<= 4;
        hex_data |= temp_hex;
    }

    return hex_data;
}

static uint8_t bit_set_pos(uint32_t data)
{
    uint8_t pos = 0;
    while (0 != (data >> 1))
    {
        pos ++;
    }

    return pos;
}

static void msg2tof(const uint8_t *data, uint8_t len)
{
    if (len >= sizeof(msg_tof_t))
    {
        const msg_tof_t *pmsg = (msg_tof_t *)data;
        /** only process mc */
        if (0 == memcmp(pmsg->type, "mc", 2))
        {
            uint8_t mask = bin2hex(pmsg->mask, 2);
            /** only enable one tag and base station */
            if (0 == (mask & (mask - 1)))
            {
                tof.range = bin2hex(pmsg->range[bit_set_pos(mask)].range, 8);
                tof.id_tag = pmsg->id_tag - '0';
                tof.id_base_station = pmsg->id_base_station - '0';
            }
        }
    }
}


/**
 * @brief altimeter receive distance task
 * @param pvParameters - task parameter
 */
static void vAltimeter(void *pvParameters)
{
    serial *pserial = pvParameters;
    TickType_t xDelay = 10 / portTICK_PERIOD_MS;
    uint8_t recv_data[sizeof(msg_tof_t) + 5];
    uint8_t *pdata = recv_data;
    char data = 0;
    uint8_t len = 0;
    uint8_t floor_cur = 0;
    uint8_t floor_prev = 0;
    float floor = 0;
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
                if (len >= (sizeof(msg_tof_t) + 5))
                {
                    break;
                }
            }

            /** receive packet */
            msg2tof(recv_data, len);
            if (!altimeter_is_calculating())
            {
                if (tof.range > 0)
                {
                    /** calculate physical floor */
                    floor = tof.range / 10.0 / board_parameter.floor_height;
                    floor_cur = (uint8_t)floor;
                    floor -= (uint8_t)floor;
                    if (floor > 0.75)
                    {
                        floor_cur ++;
                    }
                    floor_cur++;
                    if (floor_cur != floor_prev)
                    {
                        /** floor changed */
                        elev_set_phy_floor(floor_cur, floor_prev);
                        floor_prev = floor_cur;
                    }
                }
            }
            recv_data[len] = 0x00;
#if DUMP_MESSAGE
            TRACE("recv data: %s\r\n", recv_data);
#endif
        }
    }
}

/**
 * @brief initialize altimeter
 */
bool altimeter_init(void)
{
    TRACE("initialize altimeter....\r\n");
    g_serial = serial_request(COM4);
    if (NULL == g_serial)
    {
        TRACE("initialize failed, can't open serial \'COM4\'\r\n");
        return FALSE;
    }
    serial_set_baudrate(g_serial, Baudrate_115200);
    serial_open(g_serial);
    xTaskCreate(vAltimeter, "altimeter", ALTIMETER_STACK_SIZE, g_serial,
                ALTIMETER_PRIORITY, NULL);
    return TRUE;
}

/**
 * @brief get current altimeter distance
 * @return current altimeter distance
 */
uint32_t altimeter_get_distance(void)
{
    return tof.range;
}
#endif