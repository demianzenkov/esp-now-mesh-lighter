#ifndef _TSK_NOW_H_
#define _TSK_NOW_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "espnow.h"
#include "espnow_ctrl.h"
#include "tsk_bsp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  BTN_EVT_USER_SHORT (1 << 0)
#define  BTN_EVT_USER_LONG  (1 << 1)

#define  NOW_HEADER_MODE        (0x00)
#define  NOW_HEADER_BRIGHTNESS  (0x01)

#define CONFIG_RETRY_NUM        5
#define CONFIG_MAX_RECV_DATA    16

class Tsk_Now
{
public:
    esp_err_t createTask();
    static void espnowEventHandler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
private:
    static void rx_loop(void *pvParameters);
    static void tx_loop(void *pvParameters);
    void wifiInit();

public:
    xQueueHandle led_brightness_queue;
    xQueueHandle led_mode_queue;

private:
    TaskHandle_t rx_task_handle = NULL;
    TaskHandle_t tx_task_handle = NULL;
    uint32_t led_mode = LED_MODE_IDLE;

    espnow_attribute_t initiator_attribute;
    espnow_attribute_t responder_attribute;
    uint32_t responder_value;
};

extern Tsk_Now tsk_now;

#ifdef __cplusplus
}
#endif


#endif  // _TSK_NOW_H_