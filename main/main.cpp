#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_utils.h"

#include "tsk_now.h"
#include "tsk_bsp.h"
#include "tsk_tof.h"

#ifdef __cplusplus
extern "C" {
#endif


void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_storage_init();

    Tsk_BSP::initGPIO();
    tsk_bsp.initWifi();
    
    tsk_bsp.createTask();
    tsk_tof.createTask();
    tsk_now.createTask();
}


#ifdef __cplusplus
}
#endif