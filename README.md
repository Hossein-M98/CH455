# CH455 Library
Library for handling CH455 LED display driver.

## Library Features
-   Support for both 7x4 and 8x4 displays
-   Support for dimming display
-   Support for scan Keypad

## Hardware Support
It is easy to port this library to any platform. But now it is ready for use in:
- ESP32 (esp-idf)

## How To Use
1. Add `CH455.h` and `CH455.c` files to your project.  It is optional to use `CH455_platform.h` and `CH455_platform.c` files (open and config `CH455_platform.h` file).
2. Initialize platform-dependent part of handler.
4. Call `CH455_Init()`.
5. Call `CH455_ConfigDisplay()` to config display.
6. Call other functions and enjoy.

## Example
<details>
<summary>Using CH455_platform files</summary>

```c
#include <stdio.h>
#include "CH455.h"
#include "CH455_platform.h"

int main(void)
{
  CH455_Handler_t Handler;

  CH455_Platform_Init(&Handler);
  CH455_Init(&Handler, CH455_DISPLAY_TYPE_8X4);
  CH455_ConfigDisplay(&Handler, CH455_DISPLAY_STATE_ON, 0, 0); // Config to max brightness

  while (1)
  {
    // Display the number 8 and Decimal Point in the SEG1 
    CH455_SetSingleDigit_HEX(&Handler, 8 | CH455_DECIMAL_POINT, 0);
  }

  CH455_DeInit(&Handler);
  return 0;
}
```
</details>


<details>
<summary>Without using CH455_platform files (esp-idf)</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "CH455.h"

#define CH455_I2C_NUM   I2C_NUM_1
#define CH455_I2C_RATE  100000
#define CH455_SCL_GPIO  GPIO_NUM_13
#define CH455_SDA_GPIO  GPIO_NUM_14

int8_t
CH455_Platform_Init(void)
{
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = CH455_SDA_GPIO;
  conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
  conf.scl_io_num = CH455_SCL_GPIO;
  conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
  conf.master.clk_speed = CH455_I2C_RATE;
  if (i2c_param_config(CH455_I2C_NUM, &conf) != ESP_OK)
    return -1;
  if (i2c_driver_install(CH455_I2C_NUM, conf.mode, 0, 0, 0) != ESP_OK)
    return -1;
  return 0;
}

int8_t
CH455_Platform_DeInit(void)
{
  i2c_driver_delete(CH455_I2C_NUM);
  gpio_reset_pin(CH455_SDA_GPIO);
  gpio_reset_pin(CH455_SCL_GPIO);
  return 0;
}

int8_t
CH455_Platform_Send(uint8_t Address, uint8_t *Data, uint8_t DataLen)
{
  i2c_cmd_handle_t CH455_i2c_cmd_handle = 0;
  Address <<= 1;
  Address &= 0xFE;

  CH455_i2c_cmd_handle = i2c_cmd_link_create();
  i2c_master_start(CH455_i2c_cmd_handle);
  i2c_master_write(CH455_i2c_cmd_handle, &Address, 1, 1);
  i2c_master_write(CH455_i2c_cmd_handle, Data, DataLen, 1);
  i2c_master_stop(CH455_i2c_cmd_handle);
  if (i2c_master_cmd_begin(CH455_I2C_NUM, CH455_i2c_cmd_handle, 1000 / portTICK_RATE_MS) != ESP_OK)
  {
    i2c_cmd_link_delete(CH455_i2c_cmd_handle);
    return -1;
  }
  i2c_cmd_link_delete(CH455_i2c_cmd_handle);
  return 0;
}

int8_t
CH455_Platform_Receive(uint8_t Address, uint8_t *Data, uint8_t DataLen)
{
  i2c_cmd_handle_t CH455_i2c_cmd_handle = 0;
  Address <<= 1;
  Address |= 0x01;

  CH455_i2c_cmd_handle = i2c_cmd_link_create();
  i2c_master_start(CH455_i2c_cmd_handle);
  i2c_master_write(CH455_i2c_cmd_handle, &Address, 1, 1);
  i2c_master_read(CH455_i2c_cmd_handle, Data, DataLen, I2C_MASTER_LAST_NACK);
  i2c_master_stop(CH455_i2c_cmd_handle);
  if (i2c_master_cmd_begin(CH455_I2C_NUM, CH455_i2c_cmd_handle, 1000 / portTICK_RATE_MS) != ESP_OK)
  {
    i2c_cmd_link_delete(CH455_i2c_cmd_handle);
    return -1;
  }
  i2c_cmd_link_delete(CH455_i2c_cmd_handle);
  return 0;
}


void app_main(void)
{
  CH455_Handler_t Handler;

  CH455_PLATFORM_LINK_INIT(&CH455_Handler, CH455_Platform_Init);
  CH455_PLATFORM_LINK_DEINIT(&CH455_Handler, CH455_Platform_DeInit);
  CH455_PLATFORM_LINK_SEND(&CH455_Handler, CH455_Platform_Send);
  CH455_PLATFORM_LINK_RECEIVE(&CH455_Handler, CH455_Platform_Receive);

  CH455_Init(&Handler, CH455_DISPLAY_TYPE_8X4);
  CH455_ConfigDisplay(&Handler, CH455_DISPLAY_STATE_ON, 0, 0); // Config to max brightness

  while (1)
  {
    // Display the number 8 and Decimal Point in the SEG1 
    CH455_SetSingleDigit_HEX(&Handler, 8 | CH455_DECIMAL_POINT, 0);
  }

  CH455_DeInit(&Handler);
}
```
</details>
