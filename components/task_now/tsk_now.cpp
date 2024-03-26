#include "tsk_now.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "rgb.h"


static const char *TAG = "TSK_NOW";

Tsk_Now tsk_now;

esp_err_t Tsk_Now::createTask()
{
    led_brightness_queue = xQueueCreate(16, sizeof(uint8_t));
    led_mode_queue = xQueueCreate(16, sizeof(uint8_t));

    BaseType_t xReturned;
    this->rx_task_handle = NULL;
    
    xReturned = xTaskCreatePinnedToCore(this->rx_loop, "rx_now_tsk", (size_t)8 * 1024, this, 5, &this->rx_task_handle, 0);
    if (xReturned != pdPASS) 
    {
        ESP_LOGE(TAG, "Error creating rx task");
        vTaskDelete(this->rx_task_handle);
        return ESP_FAIL;
    }

    xReturned = xTaskCreatePinnedToCore(this->tx_loop, "tx_now_tsk", (size_t)16 * 1024, this, 5, &this->tx_task_handle, 0);
    if (xReturned != pdPASS) 
    {
        ESP_LOGE(TAG, "Error creating tx task");
        vTaskDelete(this->tx_task_handle);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "NOW tasks created.");
    return ESP_OK;
}

void Tsk_Now::wifiInit()
{
    ESP_ERROR_CHECK(esp_netif_init());

    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void Tsk_Now::tx_loop(void *pvParameters)
{
    Tsk_Now *p_this = (Tsk_Now *)pvParameters;

    uint8_t tx_data[2];

    espnow_frame_head_t frame_head = {
        .broadcast        = true,
        .retransmit_count = CONFIG_RETRY_NUM,
    };

    esp_err_t ret  = ESP_OK;

    while (1) {
        uint8_t mode;
        if(xQueueReceive(p_this->led_mode_queue, &mode, 1) == pdPASS) {
            tx_data[0] = NOW_HEADER_MODE;
            tx_data[1] = mode;
            ret = espnow_send(ESPNOW_TYPE_DATA, ESPNOW_ADDR_BROADCAST, tx_data, sizeof(tx_data), &frame_head, portMAX_DELAY);
            ESP_ERROR_CONTINUE(ret != ESP_OK, "<%s> espnow_send", esp_err_to_name(ret));
            tsk_bsp.setLEDMode(p_this->led_mode);
        }

        uint8_t brightness;
        if(xQueueReceive(p_this->led_brightness_queue, &brightness, 1) == pdPASS) {
            tx_data[0] = NOW_HEADER_BRIGHTNESS;
            tx_data[1] = brightness;
            ret = espnow_send(ESPNOW_TYPE_DATA, ESPNOW_ADDR_BROADCAST, tx_data, sizeof(tx_data), &frame_head, portMAX_DELAY);
            ESP_ERROR_CONTINUE(ret != ESP_OK, "<%s> espnow_send", esp_err_to_name(ret));
        }
    }
}

void Tsk_Now::rx_loop(void *pvParameters)
{
    Tsk_Now *p_this = (Tsk_Now *)pvParameters;

    // p_this->wifiInit();
    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_config.qsize.data      = 64;
    espnow_init(&espnow_config);

    uint8_t recv_data[CONFIG_MAX_RECV_DATA];
    uint8_t recv_addr[ESPNOW_ADDR_LEN] = {0};
    wifi_pkt_rx_ctrl_t recv_ctrl;
    uint32_t recv_size = 0;

    while (1) {
        esp_err_t ret = espnow_recv(ESPNOW_TYPE_DATA, recv_addr, recv_data, &recv_size, &recv_ctrl, 10);
        if(ret == ESP_OK) {
            if(recv_data[0] == NOW_HEADER_MODE) {
                p_this->led_mode = recv_data[1];
                xQueueSend(tsk_bsp.led_mode_queue, &recv_data[1], 10);
            }
            else if(recv_data[0] == NOW_HEADER_BRIGHTNESS) {
                xQueueSend(tsk_bsp.led_brightness_queue, &recv_data[1], 10);
            }
        }
    }
}
    