#ifndef RTC_EXT_H
#define RTC_EXT_H
#include <esp_err.h>
#include <driver/i2c_master.h>
#include <stdbool.h>
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RTC_EXT_DS3231_ADDR  0x68
#define RTC_EXT_PCF8563_ADDR 0x51
typedef enum rtc_ext_type_t {
    RTC_EXT_TYPE_UNKNOWN = 0,
    RTC_EXT_TYPE_DS3231,
    RTC_EXT_TYPE_PCF8563,
} rtc_ext_type_t;
typedef struct rtc_ext_dev_s *rtc_ext_handle_t;
esp_err_t rtc_ext_init(i2c_master_bus_handle_t i2c_bus, rtc_ext_handle_t *out_handle);
void rtc_ext_deinit(rtc_ext_handle_t handle);
rtc_ext_type_t rtc_ext_get_type(rtc_ext_handle_t handle);
const char *rtc_ext_type_name(rtc_ext_type_t type);
esp_err_t rtc_ext_get_time(rtc_ext_handle_t handle, struct tm *out_time, bool *out_valid);
esp_err_t rtc_ext_set_time(rtc_ext_handle_t handle, const struct tm *time);
esp_err_t rtc_ext_get_temperature(rtc_ext_handle_t handle, float *out_temp_c);
#ifdef __cplusplus
}
#endif
#endif
