/**
 * @file bus_manager.c
 * @brief I2C and SPI Bus Arbitration Implementation (ESP-IDF 6.1)
 */

#include "bus_manager.h"
#include <esp_log.h>
#include <sdkconfig.h>

#define TAG "BusManager"

static i2c_master_bus_handle_t s_i2c_bus = NULL;
static SemaphoreHandle_t s_i2c_mutex = NULL;
static SemaphoreHandle_t s_spi2_mutex = NULL;
static bool s_spi2_initialized = false;
static SemaphoreHandle_t s_spi3_mutex = NULL;
static bool s_spi3_initialized = false;

esp_err_t bus_manager_init_i2c(gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t clk_speed_hz)
{
    if (s_i2c_bus != NULL) {
        ESP_LOGD(TAG, "I2C Bus already initialized.");
        return ESP_OK;
    }

    if (s_i2c_mutex == NULL) {
        s_i2c_mutex = xSemaphoreCreateMutex();
        if (s_i2c_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create I2C mutex");
            return ESP_ERR_NO_MEM;
        }
    }

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, &s_i2c_bus);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "I2C Master Bus initialized successfully on SDA: %d, SCL: %d, Speed: %lu Hz",
             sda_pin, scl_pin, (unsigned long)clk_speed_hz);
    return ESP_OK;
}

i2c_master_bus_handle_t bus_manager_get_i2c_bus(void)
{
    return s_i2c_bus;
}

void bus_manager_set_i2c_bus(i2c_master_bus_handle_t bus)
{
    s_i2c_bus = bus;
    ESP_LOGI(TAG, "I2C Master Bus handle updated externally");
}

esp_err_t bus_manager_add_i2c_device(const i2c_device_config_t *dev_cfg, i2c_master_dev_handle_t *dev_handle)
{
    if (s_i2c_bus == NULL) {
        ESP_LOGE(TAG, "Cannot add I2C device: bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    if (!dev_cfg || !dev_handle) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!bus_manager_i2c_lock(pdMS_TO_TICKS(500))) {
        ESP_LOGE(TAG, "Timeout waiting for I2C lock to add device (addr 0x%02x)", dev_cfg->device_address);
        return ESP_ERR_TIMEOUT;
    }

    esp_err_t ret = i2c_master_bus_add_device(s_i2c_bus, dev_cfg, dev_handle);
    bus_manager_i2c_unlock();
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGD(TAG, "I2C Device registered at address 0x%02X", dev_cfg->device_address);
    return ret;
}

bool bus_manager_i2c_lock(TickType_t wait_ticks)
{
    if (s_i2c_mutex == NULL) return false;
    return (xSemaphoreTake(s_i2c_mutex, wait_ticks) == pdTRUE);
}

void bus_manager_i2c_unlock(void)
{
    if (s_i2c_mutex != NULL) {
        xSemaphoreGive(s_i2c_mutex);
    }
}

esp_err_t bus_manager_init_spi(spi_host_device_t host_id,
                               gpio_num_t mosi_pin,
                               gpio_num_t miso_pin,
                               gpio_num_t sclk_pin,
                               int max_transfer_sz)
{
    if (host_id == SPI2_HOST && s_spi2_initialized) {
        ESP_LOGD(TAG, "SPI2 Host already initialized.");
        return ESP_OK;
    }
    if (host_id == SPI3_HOST && s_spi3_initialized) {
        ESP_LOGD(TAG, "SPI3 Host already initialized.");
        return ESP_OK;
    }

    if (host_id == SPI2_HOST && s_spi2_mutex == NULL) {
        s_spi2_mutex = xSemaphoreCreateMutex();
        if (s_spi2_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create SPI2 mutex");
            return ESP_ERR_NO_MEM;
        }
    } else if (host_id == SPI3_HOST && s_spi3_mutex == NULL) {
        s_spi3_mutex = xSemaphoreCreateMutex();
        if (s_spi3_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create SPI3 mutex");
            return ESP_ERR_NO_MEM;
        }
    }

    spi_bus_config_t buscfg = {
        .mosi_io_num = mosi_pin,
        .miso_io_num = miso_pin,
        .sclk_io_num = sclk_pin,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = max_transfer_sz > 0 ? max_transfer_sz : 4096,
    };

    esp_err_t ret = spi_bus_initialize(host_id, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(ret);
    }

    if (host_id == SPI2_HOST) {
        s_spi2_initialized = true;
    } else if (host_id == SPI3_HOST) {
        s_spi3_initialized = true;
    }

    ESP_LOGI(TAG, "SPI Host %d initialized (MOSI: %d, MISO: %d, SCLK: %d)",
             host_id, mosi_pin, miso_pin, sclk_pin);
    return ESP_OK;
}

bool bus_manager_spi_lock(spi_host_device_t host_id, TickType_t wait_ticks)
{
    if (host_id == SPI2_HOST && s_spi2_mutex != NULL) {
        return (xSemaphoreTake(s_spi2_mutex, wait_ticks) == pdTRUE);
    } else if (host_id == SPI3_HOST && s_spi3_mutex != NULL) {
        return (xSemaphoreTake(s_spi3_mutex, wait_ticks) == pdTRUE);
    }
    return true;
}

void bus_manager_spi_unlock(spi_host_device_t host_id)
{
    if (host_id == SPI2_HOST && s_spi2_mutex != NULL) {
        xSemaphoreGive(s_spi2_mutex);
    } else if (host_id == SPI3_HOST && s_spi3_mutex != NULL) {
        xSemaphoreGive(s_spi3_mutex);
    }
}

esp_err_t bus_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing System Bus Manager & Resource Arbiters...");

    if (s_i2c_mutex == NULL) {
        s_i2c_mutex = xSemaphoreCreateMutex();
    }
    if (s_spi2_mutex == NULL) {
        s_spi2_mutex = xSemaphoreCreateMutex();
    }
    if (s_spi3_mutex == NULL) {
        s_spi3_mutex = xSemaphoreCreateMutex();
    }

    // Deferred I2C init: I2C will be initialized by the active board (e.g., CustomN16R8Board)
    // using bus_manager_init_i2c to prevent premature initialization on I2C_NUM_0.

    return ESP_OK;
}
