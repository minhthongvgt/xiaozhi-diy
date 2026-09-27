import sys, re
sys.stdout.reconfigure(encoding='utf-8')
txt = open('main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc', encoding='utf-8').read()

warnings = '''
#if defined(CONFIG_ENABLE_PERIPH_IR)
        ESP_LOGW(TAG, "IR driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_INA2XX)
        ESP_LOGW(TAG, "INA2xx driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_STORAGE_SDCARD) || defined(CONFIG_CUSTOM_ENABLE_SDCARD)
        ESP_LOGW(TAG, "SD Card driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_PERIPH_LED_DRIVER_IC)
        ESP_LOGW(TAG, "LED Driver IC (AW9523B/IS31FL3731) is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_CUSTOM_TOUCH) && !defined(CONFIG_CUSTOM_TOUCH_NONE)
        // Wait, touch is already partially implemented, so skip.
#endif
#if defined(CONFIG_ENABLE_SENSOR_DS18B20) || defined(CONFIG_CUSTOM_SENSOR_DS18B20)
        ESP_LOGW(TAG, "DS18B20 Temperature sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_FLOW) || defined(CONFIG_CUSTOM_SENSOR_FLOW)
        ESP_LOGW(TAG, "Water Flow sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_MQ) || defined(CONFIG_CUSTOM_SENSOR_MQ) || defined(CONFIG_CUSTOM_SENSOR_GAS)
        ESP_LOGW(TAG, "MQ/Gas sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_VIBRATION) || defined(CONFIG_CUSTOM_SENSOR_VIBRATION)
        ESP_LOGW(TAG, "Vibration sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_FLAME) || defined(CONFIG_CUSTOM_SENSOR_FLAME)
        ESP_LOGW(TAG, "Flame sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_PERIPH_RC522) || defined(CONFIG_CUSTOM_PERIPH_RC522)
        ESP_LOGW(TAG, "RFID RC522 driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_PERIPH_TP4056) || defined(CONFIG_CUSTOM_BATTERY_MONITOR_TP4056) || defined(CONFIG_CUSTOM_PERIPH_BATTERY_CHRG_PIN)
        ESP_LOGW(TAG, "TP4056 / Charge Pin monitoring is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_DHT) || defined(CONFIG_CUSTOM_SENSOR_DHT)
        ESP_LOGW(TAG, "DHT Temperature/Humidity sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_HCSR04) || defined(CONFIG_CUSTOM_SENSOR_HCSR04)
        ESP_LOGW(TAG, "HC-SR04 Ultrasonic sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_PIR) || defined(CONFIG_CUSTOM_SENSOR_PIR)
        ESP_LOGW(TAG, "PIR Motion sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_BMP280) || defined(CONFIG_CUSTOM_SENSOR_BMP280)
        ESP_LOGW(TAG, "BMP280 Pressure sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_BH1750) || defined(CONFIG_CUSTOM_SENSOR_BH1750)
        ESP_LOGW(TAG, "BH1750 Light sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_BATTERY_BQ27220) || defined(CONFIG_CUSTOM_BATTERY_MONITOR_BQ27220)
        ESP_LOGW(TAG, "BQ27220 Battery gauge driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_TOUCH_SLIDER) || defined(CONFIG_CUSTOM_TOUCH_SLIDER)
        ESP_LOGW(TAG, "Touch Slider driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_APDS9960) || defined(CONFIG_CUSTOM_SENSOR_APDS9960)
        ESP_LOGW(TAG, "APDS9960 Gesture/Color sensor driver is not yet implemented.");
#endif
#if defined(CONFIG_ENABLE_SENSOR_VL53LX) || defined(CONFIG_CUSTOM_SENSOR_VL53LX)
        ESP_LOGW(TAG, "VL53L0X/VL53L1X ToF sensor driver is not yet implemented.");
#endif
'''

idx = txt.find('void InitializeUnimplementedPeripherals() {')
if idx != -1:
    end_idx = txt.find('}', idx)
    txt = txt[:end_idx] + warnings + txt[end_idx:]

with open('main/boards/esp32s3-n16r8-custom/custom_n16r8_board.cc', 'w', encoding='utf-8') as f:
    f.write(txt)

print("Patched Task D")
