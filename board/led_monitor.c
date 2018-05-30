/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "led_monitor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "ledstatus.h"
#include "keymap.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ledmtl]"

#define LED_INTERVAL            (200)
#define LED_MONITOR_INTERVAL    (LED_INTERVAL / portTICK_PERIOD_MS)

static uint16_t led_status = 0;

/* led password */
uint8_t led_pwd[5];

typedef struct
{
    uint8_t pwd;
    uint32_t time;
}pwd_node;

static pwd_node pwds[4] = 
{
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0}
};

/* led state */ 
#define LED_STATE_NORMAL     0
#define LED_STATE_PWD        1
static uint8_t led_state = LED_STATE_NORMAL;
#define LED_PWD_CHECK_TIME 3000

/**
 * @brief check if floor arrived, 1->0 means arrive
 * @return check result
 */
static bool __INLINE is_floor_arrive(uint16_t origin, uint16_t new)
{
    return (0 != (origin & new));
}

/**
 * @brief get 1 bit position
 * @param data - data to process
 */
static uint8_t bit_to_pos(uint16_t data)
{
    uint8_t pos = 0;
    while (0 == (data & 0x01))
    {
        data >>= 1;
        pos ++;
    }

    return pos;
}

/**
 * @brief push password node to array
 * @param node - password node
 */
static void push_pwd_node(const pwd_node *node)
{
    TRACE("push pwd node: key(%d), time(%d)\r\n", node->pwd, node->time);
    for (int i = 0; i < 3; ++i)
    {
        pwds[i].pwd = pwds[i + 1].pwd;
        pwds[i].time = pwds[i + 1].time;
    }
    pwds[3].pwd = node->pwd;
    pwds[3].time = node->time;

    /* validate time */
    if (pwds[3].time > pwds[0].time)
    {
        if ((pwds[3].time - pwds[0].time) * LED_INTERVAL < LED_PWD_CHECK_TIME)
        {
            /* validate password */
            for (int i = 0; i < 3; ++i)
            {
                if (pwds[i].pwd != led_pwd[i])
                {
                    return ;
                }
            }
            floor_set_first();
        }
    }
}

/**
 * @brief led monitor task
 * @param pvParameters - task parameter
 */
static void vLedMonitor(void *pvParameters)
{
    uint16_t cur_status = 0;
    uint16_t changed_status = 0;
    uint16_t per_changed_bit = 0;
    char floor = 0;
    led_status = ledstatus_get();
    uint32_t timestamp = 0;
    for (;;)
    {
        cur_status = ledstatus_get();
        changed_status = cur_status ^ led_status;
        if (0 != changed_status)
        {
            /* led status changed */
            do
            {
                per_changed_bit = changed_status & ~(changed_status - 1);
                floor = keymap_key_to_floor(bit_to_pos(per_changed_bit));
                if (INVALID_FLOOR != floor)
                {
                    if (is_floor_arrive(led_status, per_changed_bit))
                    {
                        /* notify floor arrived */
                        floor_arrived(floor);
                    }
                    else
                    {
                        /* push password */
                        pwd_node node = {(uint8_t)floor, timestamp};
                        push_pwd_node(&node);
                    }
                }
                changed_status &= changed_status - 1;
            }while (0 != per_changed_bit);
            led_status = cur_status;
        }
        vTaskDelay(LED_MONITOR_INTERVAL);
        timestamp ++;
    }
}

/**
 * @brief initialize led monitor
 * @return init status
 */
bool led_monitor_init(void)
{
    param_get_pwd(led_pwd);
    xTaskCreate(vLedMonitor, "ledmonitor", LED_MONITOR_STACK_SIZE, NULL, 
                    LED_MONITOR_PRIORITY, NULL);

    return TRUE;
}

