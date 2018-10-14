/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include <string.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "global.h"
#include "trace.h"
#include "serial.h"
#include "parameter.h"
#include "stm32f10x_cfg.h"
#include "dbgserial.h"
#include "config.h"
#include "crc.h"
#include "parameter.h"

#undef __TRACE_MODULE
#define __TRACE_MODULE  "[license]"

#define LICENSE_MONITOR_INTERVAL     (60000 / portTICK_PERIOD_MS)
#define DEFAULT_LICENSE_TIME         (30 * 24 * 60)


#if USE_SIMPLE_LICENSE
static uint32_t g_count = 0;
#else
license_t license;
/** run time, unit is minutes */
static uint32_t total_time = 0;
static uint32_t run_time = 0;

#define KEY_LEN      16
/** 8 bytes time and 2 bytes validate(key[9],key[4],key[12],key[2],key[5],key[15],key[10],key[13]) */
#define TIME_LEN     10
/** use chip id as key */
static uint8_t default_random[] = {0xde, 0xad, 0xbe, 0xef};
static uint8_t random_key = 0xae;
static uint8_t key[KEY_LEN];
static uint8_t serial_number[KEY_LEN];

/**
 * @brief reorder data
 * @param data - data to reorder
 */
static void reorder_data(uint8_t *data, uint32_t data_len)
{
    /** out-of-order key, odd-evev, even-odd */
    uint8_t temp = 0;
    for (uint8_t i = 0; i < data_len / 2; ++i)
    {
        temp = data[i];
        if (0 != ((i + 1) % 2))
        {
            data[i] = data[i + data_len / 2 + 1];
            data[i + data_len / 2 + 1] = temp;
        }
        else
        {
            data[i] = data[i + data_len / 2 - 1];
            data[i + data_len / 2 - 1] = temp;
        }
    }
}

/**
 * @brief generate key from chip id
 * @param chip_id - chip id
 * @param pkey - generated key
 */
void license_generate_key(const uint8_t *chip_id, uint8_t *pkey)
{
    /** fill key */
    pkey[0] = chip_id[0];
    pkey[2] = chip_id[1];
    pkey[3] = chip_id[2];
    pkey[4] = chip_id[3];
    pkey[5] = chip_id[4];
    pkey[6] = chip_id[5];
    pkey[8] = chip_id[6];
    pkey[9] = chip_id[7];
    pkey[11] = chip_id[8];
    pkey[12] = chip_id[9];
    pkey[14] = chip_id[10];
    pkey[15] = chip_id[11];

    /** fill random */
    if (param_has_license())
    {
        pkey[1] = license.random[2];
        pkey[7] = license.random[1];
        pkey[10] = license.random[0];
        pkey[13] = license.random[3];
    }
    else
    {
        pkey[1] = default_random[2];
        pkey[7] = default_random[1];
        pkey[10] = default_random[0];
        pkey[13] = default_random[3];
        license.random[0] = default_random[0];
        license.random[1] = default_random[1];
        license.random[2] = default_random[2];
        license.random[3] = default_random[3];
    }

    reorder_data(pkey, KEY_LEN);
}

/**
 * @brief generate serial number from key
 * @param pkey - key generate from chip id
 * @param pserial - generated serial number
 */
void key_to_serial_number(const uint8_t *pkey, uint8_t *pserial)
{
    /** fill serial number with key */
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        pserial[i] = pkey[i];
    }

    /** use serial_number[14] to xor serial_number*/
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        if (14 != i)
        {
            pserial[i] ^= pserial[14];
        }
    }

    /** xor random ro hide serial_number[14] */
    pserial[14] ^= random_key;

    reorder_data(pserial, KEY_LEN);
}

/**
 * @brief convert serial number to key
 * @param pserial - serial number
 * @param pkey - converted key
 */
static void serial_number_to_key(const uint8_t *pserial, uint8_t *pkey)
{
    /** fill key with serial_number */
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        pkey[i] = pserial[i];
    }

    reorder_data(pkey, KEY_LEN);

    /** xor random to show key[14] */
    pkey[14] ^= random_key;

    /** use key[14] to xor key */
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        if (14 != i)
        {
            pkey[i] ^= pkey[14];
        }
    }
}

/**
 * @brief encrypt time
 * @param time - time value to encrypt
 * @param pserial - serial number
 * @param pencrypt_time - encrypted time
 */
void encrypt_time(uint32_t time, const uint8_t *pserial, uint8_t *pencrypt_time)
{
    *(uint32_t *)pencrypt_time = time;
    uint8_t temp_key[KEY_LEN];
    serial_number_to_key(pserial, temp_key);

    /** fill encrypt key */
    for (uint8_t i = 0; i < 4; ++i)
    {
        for (uint8_t j = 0; j < KEY_LEN; ++j)
        {
            if (j != i)
            {
                pencrypt_time[i] ^= temp_key[j];
            }
        }
    }

    uint8_t sum = 0;
    for (uint8_t i = 0; i < 4; ++i)
    {
        sum += pencrypt_time[i];
    }

    /** add key verify data */
    for (uint8_t i = 4; i < KEY_LEN; ++i)
    {
        pencrypt_time[i] = crc16(temp_key, KEY_LEN - i);

        /** encrypt verify data */
        pencrypt_time[i] ^= sum;
    }

    reorder_data(pencrypt_time, KEY_LEN);
}

/**
 * @brief decrypt time
 * @param pencryt_time - time need to decrypt
 * @param pserial - serial number
 * @param ptime - decrypted time
 * @param decrypt status
 */
bool decrypt_time(const uint8_t *pencrypt_time, const uint8_t *pserial, uint32_t *ptime)
{

    uint8_t encrypt_time[KEY_LEN];
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        encrypt_time[i] = pencrypt_time[i];
    }

    uint8_t temp_key[KEY_LEN];
    serial_number_to_key(pserial, temp_key);

    reorder_data(encrypt_time, KEY_LEN);

    uint8_t sum = 0;
    for (uint8_t i = 0; i < 4; ++i)
    {
        sum += encrypt_time[i];
    }

    /** verify key */
    for (uint8_t i = 4; i < KEY_LEN; ++i)
    {
        /** decrypt verify data */
        encrypt_time[i] ^= sum;

        if (encrypt_time[i] != (crc16(temp_key, KEY_LEN - i) & 0xff))
        {
            return FALSE;
        }
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        for (uint8_t j = 0; j < KEY_LEN; ++j)
        {
            if (j != i)
            {
                encrypt_time[i] ^= temp_key[j];
            }
        }
    }

    *ptime = *((uint32_t *)encrypt_time);

    return TRUE;
}

/**
 * @brief set license
 * @param license - license number
 * @return license set status
 */
bool license_set(const uint8_t *plicense)
{
    uint32_t time = 0;
    bool ret = FALSE;
    if (decrypt_time(plicense, serial_number, &time))
    {
        if (time == total_time)
        {
            TRACE("same license!\r\n");
        }
        else
        {
            memcpy(license.license, plicense, KEY_LEN);
            encrypt_time(0, serial_number, license.run_time);
            param_set_license(&license);
            ret = TRUE;
        }
    }
    else
    {
        TRACE("license invalid!!!\r\n");
    }

    return ret;
}



void generate_serial_and_key(void)
{
    /** generate key and serial number */
    uint8_t chip_id[12];
    Get_ChipID(chip_id, NULL);
    license_generate_key(chip_id, key);
    key_to_serial_number(key, serial_number);
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
    if (g_count > DEFAULT_LICENSE_TIME)
    {
        /* license expired */
        TRACE("license expired!\r\n");
        reset_param();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        SCB_SystemReset();
    }
#else
    run_time ++;
    encrypt_time(run_time, serial_number, license.run_time);
    param_set_license(&license);

    if (run_time > total_time)
    {
        /* license expired */
        TRACE("license expired!\r\n");
        for (uint8_t i = 0; i < 4; ++i)
        {
            license.random[i] = license.run_time[i];
        }

        /** regenerate current license and run time use new serial number */
        generate_serial_and_key();
        encrypt_time(total_time, serial_number, license.license);
        encrypt_time(run_time, serial_number, license.run_time);

        param_set_license(&license);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
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
    //reset_license();
    //while(1);
#if !USE_SIMPLE_LICENSE
    license = param_get_license();
    generate_serial_and_key();

    TRACE("serial number = ");
    for (uint8_t i = 0; i < KEY_LEN; ++i)
    {
        dbg_putchar("0123456789abcdef"[serial_number[i] >> 4]);
        dbg_putchar("0123456789abcdef"[serial_number[i] & 0x0f]);
    }
    dbg_putchar('\r');
    dbg_putchar('\n');

    if (!param_has_license())
    {
        TRACE("set default license\r\n");
        total_time = DEFAULT_LICENSE_TIME;
        run_time = 0;
        encrypt_time(total_time, serial_number, license.license);
        encrypt_time(run_time, serial_number, license.run_time);
        param_set_license(&license);
        TRACE("time = %d-%d\r\n", run_time, total_time);
    }
    else
    {
        if (!decrypt_time(license.license, serial_number, &total_time))
        {
            TRACE("invalid license!\r\n");
            return FALSE;
        }

        if (!decrypt_time(license.run_time, serial_number, &run_time))
        {
            TRACE("invalid license!!\r\n");
            return FALSE;
        }

        TRACE("time = %d-%d\r\n", run_time, total_time);
        if (run_time > total_time)
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
