/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include <string.h>
#include "pinconfig.h"
#include "stm32f10x_cfg.h"

/* pin configure structure */
typedef struct
{
    char name[16];
    GPIO_Group group;
    GPIO_Config config;
} PIN_CONFIG;


typedef enum
{
    AHB,
    APB1,
    APB2,
} PIN_BUS;

typedef struct
{
    PIN_BUS bus;
    uint32_t reset_reg;
    uint32_t enable_reg;
} PIN_CLOCK;


/* pin arrays */
PIN_CONFIG pins[] =
{
    {"KEY_DATA", GPIOC, 6, GPIO_Speed_2MHz, GPIO_Mode_Out_PP},
    {"KEY_ST", GPIOC, 7, GPIO_Speed_2MHz, GPIO_Mode_Out_PP},
    {"KEY_SH", GPIOC, 8, GPIO_Speed_2MHz, GPIO_Mode_Out_PP},
    {"LED_DATA", GPIOC, 13, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},
    {"LED_ST", GPIOC, 15, GPIO_Speed_2MHz, GPIO_Mode_Out_PP},
    {"LED_SH", GPIOC, 14, GPIO_Speed_2MHz, GPIO_Mode_Out_PP},
    {"SWITCH1", GPIOB, 12, GPIO_Speed_2MHz, GPIO_Mode_IPD},
    {"SWITCH2", GPIOB, 13, GPIO_Speed_2MHz, GPIO_Mode_IPD},
    {"MODE_SWITCH", GPIOB, 14, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},
    {"DIS_CALC", GPIOB, 15, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},
    {"WLAN_TX", GPIOA, 9, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
    {"WLAN_RX", GPIOA, 10, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},
    {"FM_WP", GPIOB, 5, GPIO_Speed_2MHz, GPIO_Mode_Out_PP},
    {"I2C1_SCL", GPIOB, 6, GPIO_Speed_2MHz, GPIO_Mode_Out_OD},
    {"I2C1_SDA", GPIOB, 7, GPIO_Speed_2MHz, GPIO_Mode_Out_OD},
    {"DEBUG_TX", GPIOB, 10, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
    {"DEBUG_RX", GPIOB, 11, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},
#if CAN_REMAP
    {"CAN_TX", GPIOB, 9, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
    {"CAN_RX", GPIOB, 8, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},
#else
    {"CAN_TX", GPIOA, 12, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
    {"CAN_RX", GPIOA, 11, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},
#endif
    {"ALTIMETER_TX", GPIOC, 10, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
    {"ALTIMETER_RX", GPIOC, 11, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},
    {"BT_TX", GPIOC, 12, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
    {"BT_RX", GPIOD, 2, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},
    {"RELAY1", GPIOC, 0, GPIO_Speed_2MHz, GPIO_Mode_Out_PP},
    {"RELAY2", GPIOC, 1, GPIO_Speed_2MHz, GPIO_Mode_Out_PP},
};

/* clock arrays */
PIN_CLOCK pin_clocks[] =
{
    {AHB, RCC_AHB_ENABLE_CRC, RCC_AHB_ENABLE_CRC},
    {APB2, RCC_APB2_RESET_AFIO, RCC_APB2_ENABLE_AFIO},
    {APB2, RCC_APB2_RESET_IOPA, RCC_APB2_ENABLE_IOPA},
    {APB2, RCC_APB2_RESET_IOPB, RCC_APB2_ENABLE_IOPB},
    {APB2, RCC_APB2_RESET_IOPC, RCC_APB2_ENABLE_IOPC},
    {APB2, RCC_APB2_RESET_IOPD, RCC_APB2_ENABLE_IOPD},
    {APB2, RCC_APB2_RESET_USART1, RCC_APB2_ENABLE_USART1},
    {APB1, RCC_APB1_RESET_USART2, RCC_APB1_ENABLE_USART2},
    {APB1, RCC_APB1_RESET_USART3, RCC_APB1_ENABLE_USART3},
    {APB1, RCC_APB1_RESET_UART4, RCC_APB1_ENABLE_UART4},
    {APB1, RCC_APB1_RESET_UART5, RCC_APB1_ENABLE_UART5},
    {APB1, RCC_APB1_RESET_TIM2, RCC_APB1_ENABLE_TIM2},
    {APB1, RCC_APB1_RESET_CAN, RCC_APB1_ENABLE_CAN},
};

/**
 * @brief get pin configuration by name
 * @param name - pin name
 * @return pin configuration
 */
static const PIN_CONFIG *get_pinconfig(const char *name)
{
    uint32_t len = sizeof(pins) / sizeof(PIN_CONFIG);
    for (uint32_t i = 0; i < len; ++i)
    {
        if (strcmp(name, pins[i].name) == 0)
        {
            return &pins[i];
        }
    }

    return NULL;
}

/**
 * @brief init pins
 */
void pin_init(void)
{
    /* config pin clocks */
    uint32_t len = sizeof(pin_clocks) / sizeof(PIN_CLOCK);
    for (uint32_t i = 0; i < len; ++i)
    {
        switch (pin_clocks[i].bus)
        {
        case AHB:
            RCC_AHBPeripClockEnable(pin_clocks[i].enable_reg, TRUE);
            break;
        case APB1:
            RCC_APB1PeriphReset(pin_clocks[i].reset_reg, TRUE);
            RCC_APB1PeriphReset(pin_clocks[i].reset_reg, FALSE);
            RCC_APB1PeripClockEnable(pin_clocks[i].enable_reg, TRUE);
            break;
        case APB2:
            RCC_APB2PeriphReset(pin_clocks[i].reset_reg, TRUE);
            RCC_APB2PeriphReset(pin_clocks[i].reset_reg, FALSE);
            RCC_APB2PeripClockEnable(pin_clocks[i].enable_reg, TRUE);
            break;
        default:
            break;
        }
    }

#if CAN_REMAP
    GPIO_PinRemap(CAN_REMAP_PB, TRUE);
#endif
    /* config pins */
    len = sizeof(pins) / sizeof(PIN_CONFIG);
    for (uint32_t i = 0; i < len; ++i)
    {
        GPIO_Setup(pins[i].group, &pins[i].config);
    }
}

/**
 * @brief set pin
 * @param name - pin name
 */
void pin_set(const char *name)
{
    const PIN_CONFIG *config = get_pinconfig(name);
    assert_param(config != NULL);
    GPIO_SetPin(config->group, config->config.pin);
}

/**
 * @brief reset pin
 * @param name - pin name
 */
void pin_reset(const char *name)
{
    const PIN_CONFIG *config = get_pinconfig(name);
    assert_param(config != NULL);
    GPIO_ResetPin(config->group, config->config.pin);
}

/**
 * @brief toggle pin
 * @param name - pin name
 */
void pin_toggle(const char *name)
{
    const PIN_CONFIG *config = get_pinconfig(name);
    assert_param(config != NULL);
    if (GPIO_ReadPin(config->group, config->config.pin) != 0)
    {
        GPIO_ResetPin(config->group, config->config.pin);
    }
    else
    {
        GPIO_SetPin(config->group, config->config.pin);
    }
}
/**
 * @brief check if pin is set
 * @param name - pin name
 */
bool is_pinset(const char *name)
{
    const PIN_CONFIG *config = get_pinconfig(name);
    assert_param(config != NULL);
    return (GPIO_ReadPin(config->group, config->config.pin) != 0);
}

/**
 * @brief get pin information
 * @param name - pin name
 * @param group - pin group
 * @param num - pin number
 */
void get_pininfo(const char *name, uint8_t *group, uint8_t *num)
{
    assert_param(name != NULL);
    const PIN_CONFIG *config = get_pinconfig(name);
    assert_param(config != NULL);
    if (NULL != group)
    {
        *group = config->group;
    }

    if (NULL != num)
    {
        *num = config->config.pin;
    }
}
