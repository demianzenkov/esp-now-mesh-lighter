#include "tsk_cli.h"
#include "esp_log.h"


static const char *TAG = "TSK_CLI";

Tsk_Cli tsk_cli;

esp_err_t Tsk_Cli::createTask()
{
   
    BaseType_t xReturned = xTaskCreatePinnedToCore(this->loop, "cli_tsk", (size_t)16 * 1024, this, 5, &this->task_handle, 0);
    if (xReturned != pdPASS) 
    {
        ESP_LOGE(TAG, "Error creating cli task");
        vTaskDelete(this->task_handle);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "NOW tasks created.");
    return ESP_OK;
}

void Tsk_Cli::loop(void *pvParameters)
{
    Tsk_Cli *p_this = (Tsk_Cli *)pvParameters;
    while (1) 
    {

    }
}