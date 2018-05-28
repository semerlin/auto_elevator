#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32f10x_cfg.h"
#include "global.h"
#include "i2c_software.h"
#include "bh1750.h"

/**
 * @brief process iic sensor data
 */
static void vI2CSensorProcess(void *pvParameters)
{
    UNUSED(pvParameters);
    const TickType_t xDelay = 700 / portTICK_PERIOD_MS;
    
    i2c *bh_i2c = i2c_request(i2c2);
    bh1750Init(bh_i2c);
    
    for(;;)
    {
        bh1750Start(bh_i2c);
        vTaskDelay(xDelay);
        
        bh1750GetLight(bh_i2c);
         
        vTaskDelay(400 / portTICK_PERIOD_MS);
    }
}



void vIICSensorSetup(void)
{
    xTaskCreate(vI2CSensorProcess, "I2CSensorProcess", 128, 
                NULL, 1, NULL);
}