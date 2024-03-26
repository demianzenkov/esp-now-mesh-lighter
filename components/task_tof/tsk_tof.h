#ifndef _TSK_TOF_H_
#define _TSK_TOF_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "vl53l0x.h"


#ifdef __cplusplus
extern "C" {
#endif

#define TOF_I2C_ADDR            (0x29)
#define TOF_SCL_PIN             GPIO_NUM_22
#define TOF_SDA_PIN             GPIO_NUM_21
#define TOF_SHT_PIN             (-1)
#define TOF_2V8_MODE            (0)
#define TOF_MEAS_PERIOD_MS      (50)
#define TOF_MAX_DISTANCE_CM     (50)

class Tsk_ToF
{
public:
    esp_err_t createTask();

private:
    static void loop(void *pvParameters);
    void initToF();
    void startToF();
    void stopToF();
    uint16_t ToFMeasure();

public:

private:
    TaskHandle_t task_handle = nullptr;
    struct vl53l0x_s * tof_config = nullptr;
    TickType_t last_tof_meas_time_ms = 0;
    uint16_t last_tof_meas_cm = 0;
};

extern Tsk_ToF tsk_tof;

#ifdef __cplusplus
}
#endif


#endif  // _TSK_TOF_H_