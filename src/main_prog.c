
#ifdef __cpluplus
#include <iostream>
#endif

#include "freertos/FreeRTOS.h"// FOR: portTICK_PERIOD_MS
#include "freertos/task.h"// FOR: vTaskDelay
#include "esp_spi_flash.h"// FOR: spi_flash_get_chip_size()
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "sdkconfig.h"

#define USE_CSTDLIB 1
#undef USE_CSTDLIB

#ifdef USE_CSTDLIB
#include <stdio.h>
#define PRINT(x) printf(x)
#define PRINT2(x,y) printf(x,y)
#else
#define PRINT(x)
#define PRINT2(x,y)
#endif

const gpio_num_t PWR_SUPPLY_GPIO=CONFIG_PWR_SUPPLY_GPIO;
#define SENSOR_GPIO CONFIG_SENSOR_GPIO
#define ADC2_SENSOR_CHANNEL ADC2_CHANNEL_5
#define SEC_TO_MICROSECS 1000000
#define MIN_TO_SECS 60
#define HOUR_TO_SECS 3600
#define DAY_TO_HOURS 24
#define TIME_MINS 2

static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static esp_err_t r;

void do_work(){
    PRINT("\nWork\n");
    gpio_pad_select_gpio(PWR_SUPPLY_GPIO);
    gpio_set_direction(PWR_SUPPLY_GPIO, GPIO_MODE_OUTPUT);
    // Power on!
    gpio_set_level(PWR_SUPPLY_GPIO, 1);

    // Prepare Pin 14 for Analog Input
    int read_raw;

    adc2_config_channel_atten( ADC2_SENSOR_CHANNEL, ADC_ATTEN_11db );
    vTaskDelay(2 * portTICK_PERIOD_MS);

    uint8_t k=0;
    while(++k<4) {
        r = adc2_get_raw( ADC2_SENSOR_CHANNEL, width, &read_raw);
        if ( r == ESP_OK ) {
            PRINT2("read_raw: %d\n", read_raw );
        } else if ( r == ESP_ERR_INVALID_STATE ) {
            PRINT2("%s: ADC2 not initialized yet.\n", esp_err_to_name(r));
        } else {
            PRINT2("%s\n", esp_err_to_name(r));
        }
        vTaskDelay( 2 * portTICK_PERIOD_MS );
    }
}

void hibernate(){
    rtc_gpio_isolate(GPIO_NUM_12);
    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_sleep_enable_timer_wakeup(TIME_MINS*MIN_TO_SECS*SEC_TO_MICROSECS);
    PRINT("\nGoing to sleep.\n");
    esp_deep_sleep_start();
}

void app_main()
{
    do_work();
    hibernate();
}
