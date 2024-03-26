#ifndef _TSK_CLI_H_
#define _TSK_CLI_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif


class Tsk_Cli
{
public:
    esp_err_t createTask();
    
private:
    static void loop(void *pvParameters);

private:
    TaskHandle_t task_handle = nullptr;
};

extern Tsk_Cli tsk_cli;

#ifdef __cplusplus
}
#endif


#endif  // _TSK_CLI_H_