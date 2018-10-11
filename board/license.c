/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "FreeRTOS.h"
#include "timers.h"
#include "global.h"
#include "trace.h"
#include "serial.h"
#include "parameter.h"
#include "stm32f10x_cfg.h"
#include "dbgserial.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[license]"

#define LICENSE_MONITOR_INTERVAL     (60000 / portTICK_PERIOD_MS)
#define DEFULT_LICENSE_TIME          (30 * 24 * 60)

#if USE_SIMPLE_LICENSE
static uint32_t g_count = 0;
#else
/** run time, unit is minutes */
uint64_t total_time = 0;
uint64_t run_time = 0;

#define KEY_LEN      16
/** 8 bytes time and 2 bytes validate(key[9],key[4],key[12],key[2],key[5],key[15],key[10],key[13]) */
#define TIME_LEN     10

/** use chip id as key */
uint8_t default_random[] = {0xde, 0xad, 0xbe, 0xef};
uint8_t random_key = 0xae;
uint8_t key[KEY_LEN];
uint8_t serial_number[KEY_LEN];

static void reorder_data(uint8_t *data)
{
    /** out-of-order key, odd-evev, even-odd */
    uint8_t temp = 0;
    for (uint8_t i = 0; i < KEY_LEN / 2; ++i)
    {
        temp = data[i];
        if (0 != ((i + 1) % 2))
        {
            data[i] = data[i + KEY_LEN / 2 + 1];
            data[i + KEY_LEN / 2 + 1] = temp;
        }
        else
        {
            data[i] = data[i + KEY_LEN / 2 - 1];
            data[i + KEY_LEN / 2 - 1] = temp;
        }
    }
}

void license_generate_key(const uint8_t *chip_id, uint8_t *pkey)
{
    /** fill key */
    for (uint8_t i = 0; i < 12; ++i)
    {
        pkey[i] = chip_id[i];
    }

    /** fill random */
    for (uint8_t i = 12; i < KEY_LEN; ++i)
    {
        pkey[i] = default_random[i - 12];
    }

    reorder_data(pkey);
}

void key_to_serial_number(const uint8_t *pkey, uint8_t *pserial)
{
    /** fill serial number with key */
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        pserial[i] = pkey[i];
    }

    /** use serial_number[2] to xor serial_number*/
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        if (2 != i)
        {
            pserial[i] ^= pserial[2];
        }
    }

    /** xor random ro hide serial_number[2] */
    pserial[2] ^= random_key;

    reorder_data(pserial);
}

static void serial_number_to_key(const uint8_t *pserial, uint8_t *pkey)
{
    /** fill key with serial_number */
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        pkey[i] = pserial[i];
    }

    reorder_data(pkey);

    /** xor random to show key[2] */
    pkey[2] ^= random_key;

    /** use key[2] to xor key */
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        if (2 != i)
        {
            pkey[i] ^= pkey[2];
        }
    }
}

void encrypt_time(uint64_t time, const uint8_t *pserial, uint8_t *pencrypt_time)
{
    *(uint64_t *)pencrypt_time = time;

    uint8_t temp_key[KEY_LEN];
    serial_number_to_key(pserial, temp_key);


    for (uint8_t i = 0; i < 8; ++i)
    {
        for (uint8_t j = 0; j < KEY_LEN; ++j)
        {
            if (j != i)
            {
                pencrypt_time[i] ^= temp_key[j];
            }
        }
    }

    pencrypt_time[8] = temp_key[9];
    pencrypt_time[9] = temp_key[4];
    pencrypt_time[10] = temp_key[12];
    pencrypt_time[11] = temp_key[2];
    pencrypt_time[12] = temp_key[5];
    pencrypt_time[13] = temp_key[15];
    pencrypt_time[14] = temp_key[10];
    pencrypt_time[15] = temp_key[13];

    reorder_data(pencrypt_time);
}

bool decrypt_time(const uint8_t *pencrypt_time, const uint8_t *pserial, uint64_t *ptime)
{

    uint8_t encrypt_time[KEY_LEN];
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        encrypt_time[i] = pencrypt_time[i];
    }

    uint8_t temp_key[KEY_LEN];
    serial_number_to_key(pserial, temp_key);

    reorder_data(encrypt_time);
    for (uint8_t i = 0; i < 8; ++i)
    {
        for (uint8_t j = 0; j < KEY_LEN; ++j)
        {
            if (j != i)
            {
                encrypt_time[i] ^= temp_key[j];
            }
        }
    }

    if ((encrypt_time[8] == temp_key[9]) &&
        (encrypt_time[9] == temp_key[4]) &&
        (encrypt_time[10] == temp_key[12]) &&
        (encrypt_time[11] == temp_key[2]) &&
        (encrypt_time[12] == temp_key[5]) &&
        (encrypt_time[13] == temp_key[15]) &&
        (encrypt_time[14] == temp_key[10]) &&
        (encrypt_time[15] == temp_key[13]))
    {
        *ptime = *((uint64_t *)encrypt_time);
        return TRUE;
    }

    return FALSE;
}

bool is_data_uninited(const uint8_t *data)
{
    uint8_t i = 0;
    for (i = 0; i < KEY_LEN; ++i)
    {
        if (0xff != data[i])
        {
            break;
        }
    }

    return (i >= KEY_LEN);
}
#endif

/**
 * @brief license check task
 * @param pvParameter - task parameter
 */
static void vLicense(void *pvParameters)
{
#if USE_SIMPLE_LICENSE
    g_count ++;
    if (g_count > DEFULT_LICENSE_TIME)
    {
        /* license expired */
        TRACE("license expired!\r\n");
        reset_param();
        SCB_SystemReset();
    }
#else
    run_time ++;
    uint8_t current_run_time[KEY_LEN];
    encrypt_time(run_time, serial_number, current_run_time);
    param_set_run_time(current_run_time);

    if (run_time > total_time + 1)
    {
        /* license expired */
        TRACE("license expired!\r\n");
        SCB_SystemReset();
    }
#endif
}

/**
 * @brief init license check
 */
bool license_init(void)
{
    TRACE("initialise license system...\r\n");
#if !USE_SIMPLE_LICENSE
    /** generate key and serial number */
    uint8_t chip_id[12];
    Get_ChipID(chip_id, NULL);
    license_generate_key(chip_id, key);
    key_to_serial_number(key, serial_number);

    /** read license */
    uint8_t license[KEY_LEN];
    if (!param_get_run_time(license))
    {
        return FALSE;
    }

    /** read current run time */
    uint8_t current_run_time[KEY_LEN];
    if (!param_get_run_time(current_run_time))
    {
        return FALSE;
    }

    if (is_data_uninited(license) && is_data_uninited(current_run_time))
    {
        /** TODO: set default license */
        total_time = 30 * 24 * 60;
        run_time = 0;
        encrypt_time(total_time, serial_number, license);
        encrypt_time(run_time, serial_number, current_run_time);
        param_set_license(license);
        param_set_run_time(current_run_time);
    }
    else
    {
        if (!decrypt_time(license, serial_number, &total_time))
        {
            TRACE("invalid license!\r\n");
            return FALSE;
        }

        if (!decrypt_time(current_run_time, serial_number, &run_time))
        {
            TRACE("invalid license!!\r\n");
            return FALSE;
        }

        if (run_time >= total_time)
        {
            TRACE("license expired!\r\n");
            return FALSE;
        }
    }
#endif

    TimerHandle_t license_tmr = xTimerCreate("license_tmr", LICENSE_MONITOR_INTERVAL, TRUE, NULL,
                                             vLicense);
    if (NULL == license_tmr)
    {
        TRACE("initialise license system failed!\r\n");
        return FALSE;
    }
    xTimerStart(license_tmr, 0);
    return TRUE;
}
