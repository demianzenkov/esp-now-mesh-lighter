#include "tsk_tof.h"
#include "tsk_bsp.h"
#include "tsk_now.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "vl53l0x.h"


static const char *TAG = "TSK_TOF";

Tsk_ToF tsk_tof;

esp_err_t Tsk_ToF::createTask()
{
    BaseType_t xReturned;
    this->task_handle = NULL;
    ESP_LOGI(TAG, "Create and start task.");
    xReturned = xTaskCreatePinnedToCore(this->loop, "tsk_tof", (size_t)8 * 1024, this, 5, &this->task_handle, 1);
    if (xReturned != pdPASS) 
    {
        ESP_LOGE(TAG, "Error creating task");
        vTaskDelete(this->task_handle);
        return ESP_FAIL;
    }
    return ESP_OK;
}

void Tsk_ToF::initToF()
{
    tof_config = vl53l0x_config(
        I2C_NUM_0, TOF_SCL_PIN, TOF_SDA_PIN, TOF_SHT_PIN, TOF_I2C_ADDR, TOF_2V8_MODE);
    if(tof_config == NULL) {
        ESP_LOGE(TAG, "Error configuring tof");
        uint8_t mode_err = LED_EVT_ERR;
        xQueueSend(tsk_bsp.led_mode_queue, &mode_err, 10);
        return;
    }
    const char * ret = vl53l0x_init(tof_config);
    if(ret != NULL) {
        ESP_LOGE(TAG, "ERROR ToF Init: %s", ret);
        uint8_t mode_err = LED_EVT_ERR;
        xQueueSend(tsk_bsp.led_mode_queue, &mode_err, 10);
    }
}

void Tsk_ToF::startToF()
{
    vl53l0x_startContinuous(tof_config, TOF_MEAS_PERIOD_MS);
}

void Tsk_ToF::stopToF()
{
    vl53l0x_stopContinuous(tof_config);
}

uint16_t Tsk_ToF::ToFMeasure()
{
    return vl53l0x_readRangeSingleMillimeters(tof_config);
    // return vl53l0x_readRangeContinuousMillimeters(tof_config);
}

void Tsk_ToF::loop(void *pvParameters)
{
    Tsk_ToF *p_this = (Tsk_ToF *)pvParameters;

    p_this->initToF();
    // p_this->startToF();

    while (1) {
        TickType_t cur_ticks = xTaskGetTickCount(); 
        if(cur_ticks - p_this->last_tof_meas_time_ms > TOF_MEAS_PERIOD_MS) {
            p_this->last_tof_meas_time_ms = cur_ticks;
            uint16_t tof_meas_mm = p_this->ToFMeasure();
            if(tof_meas_mm == 65535) {
                ESP_LOGE(TAG, "ToF TIMEOUT");
                continue;
            }

            uint16_t tof_meas_cm = tof_meas_mm / 10;
            if((tof_meas_cm != p_this->last_tof_meas_cm) && (tof_meas_cm != 0)) {
                if(tof_meas_cm < TOF_MAX_DISTANCE_CM) {
                    ESP_LOGI(TAG, "tof сm: %d", tof_meas_cm);
                    p_this->last_tof_meas_cm = tof_meas_cm;
                    xQueueSend(tsk_now.led_brightness_queue, &p_this->last_tof_meas_cm, 10);
                    xQueueSend(tsk_bsp.led_brightness_queue, &p_this->last_tof_meas_cm, 10);
                }
            }
        } 
    }
}
    

void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(tsk_bsp.gpio_evt_queue, &gpio_num, NULL);
}
