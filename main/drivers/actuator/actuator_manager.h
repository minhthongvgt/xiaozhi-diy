#ifndef ACTUATOR_MANAGER_H
#define ACTUATOR_MANAGER_H
#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t actuator_manager_init(void);
void actuator_set_relay(bool state);
bool actuator_get_relay(void);
void actuator_set_servo_angle(uint8_t angle_deg);
uint8_t actuator_get_servo_angle(void);
void actuator_servo_detach(void);
bool actuator_is_servo_attached(void);
void actuator_set_servo_pulse_range(uint16_t min_us, uint16_t max_us);
void actuator_set_dc_motor(int motor_id, int speed_pct);
void actuator_beep(uint32_t freq_hz, uint32_t duration_ms);
void actuator_vibrate(uint32_t duration_ms);
#ifdef __cplusplus
}
#endif
#endif 
