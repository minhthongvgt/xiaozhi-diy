/**
 * @file actuator_manager.h
 * @brief Actuators Subsystem Manager (Relay, Servo, DC Motor, PCA9685, Buzzer, Haptic)
 */

#ifndef ACTUATOR_MANAGER_H
#define ACTUATOR_MANAGER_H

#include <esp_err.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all configured actuator peripherals
 */
esp_err_t actuator_manager_init(void);

/**
 * @brief Set Relay state (ON / OFF)
 */
void actuator_set_relay(bool state);

/**
 * @brief Set Servo position (0 to 180 degrees)
 */
void actuator_set_servo_angle(uint8_t angle_deg);

/**
 * @brief Set DC Motor speed and direction (-100 to +100)
 */
void actuator_set_dc_motor(int motor_id, int speed_pct);

/**
 * @brief Trigger buzzer beep
 */
void actuator_beep(uint32_t freq_hz, uint32_t duration_ms);

/**
 * @brief Trigger haptic feedback vibration
 */
void actuator_vibrate(uint32_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif // ACTUATOR_MANAGER_H
