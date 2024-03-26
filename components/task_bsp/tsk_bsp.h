#ifndef _TSK_BSP_H_
#define _TSK_BSP_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "led_strip.h"
#include "vl53l0x.h"
#include "rgb.h"

#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LDO_ENABLE_PIN          GPIO_NUM_19
#define LED_STRIP_EN_PIN        GPIO_NUM_13
#define LED_STRIP_PIN           GPIO_NUM_23

#define USER_BUTTON_PIN         GPIO_NUM_12
#define USER_BUTTON_PIN_SEL     (1 << USER_BUTTON_PIN)

#define OUTPUT_PIN_SEL          ((1 << LDO_ENABLE_PIN) | (1 << LED_STRIP_EN_PIN))

#define LED_STRIP_NUM_LEDS      (3)
#define LED_TYPE                LED_STRIP_SK6812


#define LED_MODE_IDLE   (0x00)
#define LED_MODE_R      (0x01)
#define LED_MODE_G      (0x02)
#define LED_MODE_B      (0x03)
#define LED_MODE_W      (0x04)
#define LED_MODE_MAX    (0x04)
#define LED_MODE_ERR    (0x05)

#define LED_EVT_R   (1 << LED_MODE_R)
#define LED_EVT_G   (1 << LED_MODE_G)
#define LED_EVT_B   (1 << LED_MODE_B)
#define LED_EVT_W   (1 << LED_MODE_W)
#define LED_EVT_ERR (1 << LED_MODE_ERR)
#define LED_EVT_SEL (LED_EVT_R | LED_EVT_G | LED_EVT_B | LED_EVT_W | LED_EVT_ERR)


class Tsk_BSP
{
public:
    esp_err_t createTask();
    static void initGPIO();
    void initWifi();

    void setLEDMode(uint8_t mode);
    void setLEDBrightness(uint8_t value);

    static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    
private:
    static void loop(void *pvParameters);
    void initLED();
    void initToF();
    void startToF();
    void stopToF();
    uint16_t ToFMeasure();
    
public:
    xQueueHandle gpio_evt_queue = NULL;
    xQueueHandle led_brightness_queue = NULL;
    xQueueHandle led_mode_queue = NULL;
    EventGroupHandle_t btn_evt_group = NULL;

private:
    TaskHandle_t task_handle = nullptr;
    led_strip_t led_strip;
    bool usr_btn_pressed = false;
    TickType_t usr_btn_pressed_ticks = 0;
    const uint8_t usr_btn_debounce_ms = 50;
    const uint16_t usr_btn_long_ms = 1000;
    uint8_t led_mode = LED_MODE_IDLE;

    esp_netif_t * ap_netif;
};

extern Tsk_BSP tsk_bsp;

#ifdef __cplusplus
}
#endif


#endif  // _TSK_BSP_H_