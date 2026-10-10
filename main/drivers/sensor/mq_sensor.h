#ifndef MQ_SENSOR_H
#define MQ_SENSOR_H
#include <esp_err.h>
#include <esp_adc/adc_oneshot.h>
#include <hal/gpio_types.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    MQ_TYPE_MQ2 = 0,    
    MQ_TYPE_MQ135 = 1,  
} mq_sensor_type_t;
typedef struct {
    gpio_num_t       pin;        
    mq_sensor_type_t type;       
    float            rl_kohm;    
    float            r0_kohm;    
} mq_sensor_config_t;
typedef struct mq_sensor_dev_t* mq_sensor_handle_t;
esp_err_t mq_sensor_init(adc_oneshot_unit_handle_t adc1_handle,
                         const mq_sensor_config_t *config,
                         mq_sensor_handle_t *out_handle);
esp_err_t mq_sensor_read(mq_sensor_handle_t handle, float *out_ppm, float *out_voltage_mv);
bool mq_sensor_is_leak_alert(mq_sensor_handle_t handle, float current_ppm);
esp_err_t mq_sensor_calibrate(mq_sensor_handle_t handle, float *out_r0);
void mq_sensor_deinit(mq_sensor_handle_t handle);
#ifdef __cplusplus
}
#endif
#endif 
