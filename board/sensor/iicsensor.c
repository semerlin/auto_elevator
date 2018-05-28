#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32f10x_cfg.h"
#include "global.h"
#include "environment.h"
#include "i2c.h"
#include "bh1750.h"

#ifdef __I2C_HARDWARE
  #define I2C_NAME  "hardware"
#else
  #define I2C_NAME  "software"
#endif


/**
 * @brief process iic sensor data
 */
static void vI2CSensorProcess(void *pvParameters)
{
    UNUSED(pvParameters);
    const TickType_t xDelay = 700 / portTICK_PERIOD_MS;
    
    i2c bhI2C;
    bhI2C.device = i2c_get_device(I2C_NAME);
    bhI2C.i2c_handle = bhI2C.device->i2c_request(i2c2);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
#ifdef __I2C_HARDWARE
    portENTER_CRITICAL();
#endif
    bh1750Init(&bhI2C);
#ifdef __I2C_HARDWARE
    portEXIT_CRITICAL();
#endif
    
    
    for(;;)
    {
        //start sample
#ifdef __I2C_HARDWARE
        portENTER_CRITICAL();
#endif
        bh1750Start(&bhI2C);
#ifdef __I2C_HARDWARE
        portEXIT_CRITICAL();
#endif
        vTaskDelay(xDelay);
        
        //get sensor data
#ifdef __I2C_HARDWARE
        portENTER_CRITICAL();
#endif
        bh1750GetLight(&bhI2C);
#ifdef __I2C_HARDWARE
        portEXIT_CRITICAL();
#endif
         
        vTaskDelay(400 / portTICK_PERIOD_MS);
    }
}



void vIICSensorSetup(void)
{
    xTaskCreate(vI2CSensorProcess, "I2CSensorProcess", 128, 
                NULL, 1, NULL);
}