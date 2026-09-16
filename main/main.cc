#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "application.h"

#define TAG "main"

extern "C" esp_err_t app_main_hardware_init(void);

extern "C" void app_main(void)
{
    // 1. Execute hardware safety validation and subsystem init
    esp_err_t hw_ret = app_main_hardware_init();
    if (hw_ret != ESP_OK) {
        ESP_LOGE(TAG, "Hardware initialization aborted due to safety errors.");
        return;
    }

    // 2. Ensure NVS flash is ready
    esp_err_t ret = nvs_flash_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "NVS flash status: %s", esp_err_to_name(ret));
    }

    // 3. Initialize and run the application
    auto& app = Application::GetInstance();
    app.Initialize();
    app.Run();  // This function runs the main event loop and never returns
}
