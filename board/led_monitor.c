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
#include "semphr.h"
#include "timers.h"
#include "queue.h"
#include "trace.h"
#include "led_status.h"
#include "elevator.h"
#include "parameter.h"
#include "global.h"
#include "boardmap.h"
#include "floormap.h"
#include "elevator.h"
#include "expand.h"
#include "protocol_expand.h"
#ifdef __MASTER
#include "robot.h"
#include "altimeter.h"
#include "altimeter_calc.h"
#endif

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[ledmtl]"

extern parameters_t board_parameter;

#define LED_INTERVAL                 (200)
#define LED_MONITOR_INTERVAL         (LED_INTERVAL / portTICK_PERIOD_MS)

#ifdef __MASTER
typedef struct
{
    uint8_t id_board;
    uint16_t prev_status;
    uint16_t cur_status;
} led_status_t;
static xQueueHandle xQueueLed = NULL;
#define LED_QUEUE_SIZE       20
#define LED_WORK_MONITOR_INTERVAL    (1000 / portTICK_PERIOD_MS)

typedef struct
{
    uint8_t pwd;
    uint32_t time;
} pwd_node;

static pwd_node pwds[PARAM_PWD_LEN] =
{
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0}
};

typedef enum
{
    NORMAL_PHASE,
    RESET_FLOOR_PHASE,
} floor_phase_t;

static uint8_t floor_err_cnt = 0;
static floor_phase_t floor_phase = NORMAL_PHASE;
#define RESET_FLOOR_COUNT_PHASE1    5
#define RESET_FLOOR_COUNT_PHASE2    10
static uint8_t floor_in_phase[RESET_FLOOR_COUNT_PHASE2];

#endif

#ifdef __MASTER
/**
 * @brief check if floor arrived, 0->1means arrive
 * @return check result
 */
static bool __INLINE is_floor_arrive(uint16_t origin_val, uint16_t new_val)
{
    return (0 == (origin_val & new_val));
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
    for (int i = 0; i < PARAM_PWD_LEN - 1; ++i)
    {
        pwds[i].pwd = pwds[i + 1].pwd;
        pwds[i].time = pwds[i + 1].time;
    }
    pwds[PARAM_PWD_LEN - 1].pwd = node->pwd;
    pwds[PARAM_PWD_LEN - 1].time = node->time;

    /* validate time */
    if (pwds[PARAM_PWD_LEN - 1].time > pwds[0].time)
    {
        if ((pwds[PARAM_PWD_LEN - 1].time - pwds[0].time) * LED_INTERVAL < (uint32_t)
            board_parameter.pwd_window * 1000)
        {
            /* validate password */
            for (int i = 0; i < PARAM_PWD_LEN; ++i)
            {
                if (pwds[i].pwd != board_parameter.pwd[i])
                {
                    return ;
                }
            }
            elev_set_floor(1, 0);
        }
    }
}
#endif

#ifdef __MASTER

static bool is_all_floor_same(void)
{
    uint8_t floor = floor_in_phase[0];
    for (uint8_t i = 0; i < RESET_FLOOR_COUNT_PHASE2; ++i)
    {
        if (floor_in_phase[i] != floor)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * @brief led monitor task
 * @param pvParameters - task parameter
 */
static void elev_pwd_go(uint8_t floor)
{
    switch (floor_phase)
    {
    case NORMAL_PHASE:
        if (floor_err_cnt > RESET_FLOOR_COUNT_PHASE1)
        {
            floor_err_cnt = 0;
            /** error maybe happened */
            TRACE("floor error maybe happened!\r\n");
            floor_phase = RESET_FLOOR_PHASE;
        }
        break;
    case RESET_FLOOR_PHASE:
        if (floor_err_cnt > RESET_FLOOR_COUNT_PHASE2)
        {
            floor_err_cnt = 0;
            /** check all floor data */
            if (is_all_floor_same())
            {
                /** error really happened */
                TRACE("floor reset to %d\r\n", floor);
                elev_set_floor(floor, 0);
                /** notify floor arrive */
                elev_arrived(floor);
            }

            floor_phase = NORMAL_PHASE;
        }
        else
        {
            floor_in_phase[floor_err_cnt - 1] = elev_floor();
        }
        break;
    default:
        break;
    }

    floor_err_cnt ++;
    elev_go(floor);
}

static void vLedWorkMonitor(void *pvParameters)
{
    /** check elevator status */
    if (work_robot == elev_state_work())
    {
        uint8_t floor = robot_checkin_get();
        if (DEFAULT_CHECKIN != floor)
        {
            if (!is_led_on(floor) && (floor != elev_floor()))
            {
                if (CALC_PWD == board_parameter.calc_type)
                {
                    elev_pwd_go(floor);
                }
                else
                {
                    elev_go(floor);
                }
            }
            else
            {
                floor_err_cnt = 0;
                floor_phase = NORMAL_PHASE;
            }
        }
    }
}

/**
 * @brief led process task
 * @param pvParameters - task parameter
 */
static void vLedProcess(void *pvParameters)
{
    led_status_t led_status;
    uint16_t changed_status = 0;
    uint16_t per_changed_bit = 0;
    char floor = 0;
    uint32_t timestamp = 0;
    for (;;)
    {
        if (xQueueReceive(xQueueLed, &led_status, portMAX_DELAY))
        {
            changed_status = led_status.prev_status ^ led_status.cur_status;
            if (0 != changed_status)
            {
                /* led status changed */
                do
                {
                    per_changed_bit = changed_status & ~(changed_status - 1);
                    floor = boardmap_key_to_floor(led_status.id_board, bit_to_pos(per_changed_bit));
                    if (INVALID_FLOOR != floor)
                    {
                        if (is_floor_arrive(led_status.prev_status, per_changed_bit))
                        {
                            TRACE("floor led off: %d\r\n", floor);

                            /** check altimter calculation */
                            if (altimeter_is_calculating())
                            {
                                altimeter_calc_once(floor);
                                if (floor < board_parameter.total_floor)
                                {
                                    elev_go(floor + 1);
                                }
                                else
                                {
                                    elev_go(1);
                                }
                            }
                            else
                            {
                                /* notify floor arrived */
                                elev_arrived(floor);
                            }
                        }
                        else
                        {
                            TRACE("floor led on: %d\r\n", floor);
                            if (CALC_PWD == board_parameter.calc_type)
                            {
                                /* push password */
                                pwd_node node = {(uint8_t)floor, timestamp};
                                push_pwd_node(&node);
                            }
                        }
                    }
                    changed_status &= changed_status - 1;
                }
                while (0 != changed_status);
            }

            if (CALC_PWD == board_parameter.calc_type)
            {
                timestamp ++;
            }
        }
    }
}

/**
 * @brief process led status
 * @param[in] id_board: board id
 * @param[in] prev_status: previous led status
 * @param[in] cur_status: current led status
 */
void led_monitor_process(uint8_t id_board, uint16_t prev_status, uint16_t cur_status)
{
    led_status_t status = {id_board, prev_status, cur_status};
    xQueueSend(xQueueLed, &status, 50 / portTICK_PERIOD_MS);
}
#endif

/**
 * @brief led monitor task
 * @param pvParameters - task parameter
 */
static void vLedMonitor(void *pvParameters)
{
    uint16_t cur_status = led_status_get();
    static bool first_time = TRUE;
#ifdef __MASTER
    static led_status_t status = {0, 0, 0};
    if (first_time)
    {
        status.id_board = board_parameter.id_board;
        status.prev_status = cur_status;
        status.cur_status = cur_status;
        boardmap_update_led_status(board_parameter.id_board, cur_status);
        first_time = FALSE;
    }
#else
    static uint16_t prev_status = 0;
#endif

#ifdef __MASTER
    status.cur_status = cur_status;
    if (status.cur_status != status.prev_status)
    {
        xQueueSend(xQueueLed, &status, 50 / portTICK_PERIOD_MS);
        boardmap_update_led_status(board_parameter.id_board, cur_status);
        status.prev_status = status.cur_status;
    }
#else
    if (is_expand_board_registered())
    {
        if (first_time)
        {
            vTaskDelay(LED_MONITOR_INTERVAL);
            notify_led_status(board_parameter.id_board, cur_status);
            prev_status = cur_status;
            first_time = FALSE;
        }
        else
        {
            if (cur_status != prev_status)
            {
                notify_led_status(board_parameter.id_board, cur_status);
                prev_status = cur_status;
            }
        }
    }
#endif
}

/**
 * @brief initialize led monitor
 * @return init status
 */
bool led_monitor_init(void)
{
    TRACE("initialize led monitor...\r\n");

    TimerHandle_t led_tmr = xTimerCreate("led_tmr", LED_MONITOR_INTERVAL, TRUE, NULL, vLedMonitor);
    if (NULL == led_tmr)
    {
        TRACE("initialise led monitor failed!\r\n");
        return FALSE;
    }
    xTimerStart(led_tmr, 0);
#ifdef __MASTER
    TimerHandle_t ledwork_tmr = xTimerCreate("ledwork_tmr", LED_WORK_MONITOR_INTERVAL, TRUE, NULL,
                                             vLedWorkMonitor);
    if (NULL == ledwork_tmr)
    {
        TRACE("initialise led monitor failed!\r\n");
        return FALSE;
    }
    xTimerStart(ledwork_tmr, 0);

    xQueueLed = xQueueCreate(LED_QUEUE_SIZE, sizeof(led_status_t));
    xTaskCreate(vLedProcess, "ledprocess", LED_PROCESS_STACK_SIZE, NULL,
                LED_PROCESS_PRIORITY, NULL);
#endif

    return TRUE;
}
