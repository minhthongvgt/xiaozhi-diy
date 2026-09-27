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
 * @brief Get Relay state
 */
bool actuator_get_relay(void);

/**
 * @brief Set Servo position (0 to 180 degrees)
 */
void actuator_set_servo_angle(uint8_t angle_deg);

/**
 * @brief Get current Servo position
 */
uint8_t actuator_get_servo_angle(void);

/**
 * @brief Detach Servo PWM signal (releases holding torque, prevents buzz/overheat/battery drain)
 */
void actuator_servo_detach(void);

/**
 * @brief Check if Servo is currently attached (PWM active)
 */
bool actuator_is_servo_attached(void);

/**
 * @brief Set custom pulse width range for servo (in microseconds)
 * @param min_us Minimum pulse width at 0 degrees (default: 500)
 * @param max_us Maximum pulse width at 180 degrees (default: 2500)
 */
void actuator_set_servo_pulse_range(uint16_t min_us, uint16_t max_us);

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
