#include "tsk_bsp.h"
#include "tsk_now.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "vl53l0x.h"

static const char *TAG = "TSK_BSP";
static void IRAM_ATTR gpio_isr_handler(void *arg);

Tsk_BSP tsk_bsp;

esp_err_t Tsk_BSP::createTask()
{
    btn_evt_group = xEventGroupCreate();
    gpio_evt_queue = xQueueCreate(1, sizeof(uint32_t));
   
    led_brightness_queue = xQueueCreate(10, sizeof(uint8_t));
    led_mode_queue = xQueueCreate(10, sizeof(uint8_t));

    BaseType_t xReturned;
    this->task_handle = NULL;
    ESP_LOGI(TAG, "Create and start task.");
    // xReturned = xTaskCreate(this->loop, "bsp_tsk", (size_t)8 * 1024, this, 5, &this->task_handle);
    xReturned = xTaskCreatePinnedToCore(this->loop, "bsp_tsk", (size_t)8 * 1024, this, 5, &this->task_handle, 1);
    if (xReturned != pdPASS) 
    {
        ESP_LOGE(TAG, "Error creating task");
        vTaskDelete(this->task_handle);
        return ESP_FAIL;
    }
    return ESP_OK;
}

void Tsk_BSP::initGPIO()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = OUTPUT_PIN_SEL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_HIGH_LEVEL,
    };
    gpio_config(&io_conf);
    gpio_set_level(LDO_ENABLE_PIN, 1);
    gpio_set_level(LED_STRIP_EN_PIN, 1);

    io_conf.pin_bit_mask = USER_BUTTON_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(USER_BUTTON_PIN, gpio_isr_handler, (void *) USER_BUTTON_PIN);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void Tsk_BSP::initWifi()
{
    ESP_ERROR_CHECK(esp_netif_init());

    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    esp_err_t err;
    ap_netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    /* Set wifi handlers */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, (void *)this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, (void *)this));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    wifi_config_t sta_wifi_config = {
        .ap = {
            "marblehands",
            "sunshine"
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &sta_wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG,"WIFI INIT OK");
}

void Tsk_BSP::wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        ESP_LOGI(TAG,"WIFI_EVENT_STA_START");
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        ESP_LOGI(TAG,"WIFI_EVENT_STA_DISCONNECTED");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ESP_LOGI(TAG,"GOT_IP");
    }

}



void Tsk_BSP::initLED()
{
    led_strip_install();

    led_strip.type = LED_TYPE;
    led_strip.length = LED_STRIP_NUM_LEDS;
    led_strip.gpio = LED_STRIP_PIN;
    led_strip.buf = NULL;
    led_strip.is_rgbw = true;
    led_strip.brightness = 10;
    led_strip.channel = RMT_CHANNEL_0;

    led_strip_init(&led_strip);
    
    led_strip_fill(&led_strip, 0, led_strip.length, {0x0F, 0x0F, 0x0F});
    led_strip_flush(&led_strip);
    vTaskDelay(pdMS_TO_TICKS(200));
    led_strip_fill(&led_strip, 0, led_strip.length, {0x00, 0x00, 0x00});
    led_strip_flush(&led_strip);
}

void Tsk_BSP::setLEDBrightness(uint8_t value) 
{
    led_strip.brightness = value;
    led_strip_flush(&led_strip);
}

void Tsk_BSP::setLEDMode(uint8_t mode)
{
    if(mode == LED_MODE_R) {
        led_strip_fill(&this->led_strip, 0, this->led_strip.length, {0xF0, 0x00, 0x00});
    } else if(mode == LED_MODE_G) {
        led_strip_fill(&this->led_strip, 0, this->led_strip.length, {0x00, 0xF0, 0x00});
    } else if(mode == LED_MODE_B) {
        led_strip_fill(&this->led_strip, 0, this->led_strip.length, {0x00, 0x00, 0xF0});
    } else if(mode == LED_MODE_W) {
        led_strip_fill(&this->led_strip, 0, this->led_strip.length, {0xF0, 0xF0, 0xF0});
    }
    else if(mode == LED_MODE_ERR) {
        rgb_t clr =  {0xF0, 0x00, 0x00};
        led_strip_set_pixels(&this->led_strip, 0, 1,  &clr);
        clr.r = 0x00;
        clr.g = 0xF0;
        led_strip_set_pixels(&this->led_strip, 1, 1,  &clr);
        clr.g = 0x00;
        clr.b = 0xF0;
        led_strip_set_pixels(&this->led_strip, 2, 1,  &clr);
        ESP_LOGW(TAG, "Pixel ERROR MODE");
    }
    else {
        return;
    }
    led_strip_flush(&this->led_strip);
}

void Tsk_BSP::loop(void *pvParameters)
{
    Tsk_BSP *p_this = (Tsk_BSP *)pvParameters;

    p_this->initLED();

    uint32_t gpio_num=0;
    EventBits_t led_ev_bits;
    EventBits_t btn_ev_bits;

    while (1) {
        TickType_t cur_ticks = xTaskGetTickCount();
        if (xQueueReceive(p_this->gpio_evt_queue, &gpio_num, 1) == pdPASS) {
            if (gpio_num == USER_BUTTON_PIN) {
                if (p_this->usr_btn_pressed == false) {
                    p_this->usr_btn_pressed = true;
                    p_this->usr_btn_pressed_ticks = cur_ticks;
                }
            }
        }
        if(p_this->usr_btn_pressed) {
            uint8_t usr_button_state = gpio_get_level(USER_BUTTON_PIN);
            if(usr_button_state) {
                TickType_t diff_ticks = cur_ticks - p_this->usr_btn_pressed_ticks;
                if((diff_ticks > p_this->usr_btn_debounce_ms) && (diff_ticks < p_this->usr_btn_long_ms)) {
                    xEventGroupSetBits(p_this->btn_evt_group, BTN_EVT_USER_SHORT);
                }
                else if (diff_ticks > p_this->usr_btn_long_ms) {
                    xEventGroupSetBits(p_this->btn_evt_group, BTN_EVT_USER_LONG);
                    
                }
                p_this->usr_btn_pressed = false;
            }
        }

        btn_ev_bits = xEventGroupWaitBits(p_this->btn_evt_group, BTN_EVT_USER_SHORT | BTN_EVT_USER_LONG, pdTRUE, pdFALSE, 1);
        if(btn_ev_bits)
        {
            if(p_this->led_mode < LED_MODE_MAX) {
                p_this->led_mode += 1;
            }
            else {
                p_this->led_mode = LED_MODE_R;
            }
            xQueueSend(tsk_now.led_mode_queue, &p_this->led_mode, 10);
            p_this->setLEDMode(p_this->led_mode);
            ESP_LOGI(TAG, "Button pressed, mode: %d", p_this->led_mode);
        }

        uint8_t brightness;
        if(xQueueReceive(p_this->led_brightness_queue, &brightness, 1) == pdPASS) {
            p_this->setLEDBrightness(brightness);
        }

        uint8_t mode;
        if(xQueueReceive(p_this->led_mode_queue, &mode, 1) == pdPASS) {
            p_this->led_mode = mode;
            p_this->setLEDMode(p_this->led_mode);
        }
    }
}
    

void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(tsk_bsp.gpio_evt_queue, &gpio_num, NULL);
}

