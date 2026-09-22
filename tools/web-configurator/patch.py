import re

with open('app.js', 'r', encoding='utf-8') as f:
    app_js = f.read()

optgroup_actuators = '<optgroup label="Actuators (Thiết bị chấp hành)">'
optgroup_sensors = '<optgroup label="Sensors (Cảm biến)">'

app_js = app_js.replace(
    optgroup_actuators,
    optgroup_actuators + '\n          <option value="actuator_pca9685">Mở rộng PWM PCA9685 (I2C)</option>'
)
app_js = app_js.replace(
    optgroup_sensors,
    optgroup_sensors + '\n          <option value="sensor_nfc">Đầu đọc thẻ RFID/NFC (I2C)</option>\n          <option value="sensor_rtc">Đồng hồ thời gian thực RTC (I2C)</option>'
)

comm_spi = '<option value="spi">Cổng SPI Bus dùng chung</option>'
app_js = app_js.replace(
    comm_spi,
    comm_spi + '\n        <option value="comm_twai">Mạng CAN/TWAI Bus</option>\n        <option value="comm_4g">Module 4G LTE (UART)</option>'
)

fields_injection = '''
      } else if (type === "sensor_nfc") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_nfc_sda">I2C SDA (hoặc SPI MOSI)</label><input type="number" class="form-control" id="modal_field_nfc_sda" value="8"></div>
          <div class="form-item"><label class="form-label" for="modal_field_nfc_scl">I2C SCL (hoặc SPI SCK)</label><input type="number" class="form-control" id="modal_field_nfc_scl" value="9"></div>
          <div class="form-item"><label class="form-label" for="modal_field_nfc_irq">Ngắt IRQ</label><input type="number" class="form-control" id="modal_field_nfc_irq" value="17"></div>
          <div class="form-item"><label class="form-label" for="modal_field_nfc_rst">Reset RST</label><input type="number" class="form-control" id="modal_field_nfc_rst" value="18"></div>
        `;
      } else if (type === "sensor_rtc") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_rtc_sda">I2C SDA</label><input type="number" class="form-control" id="modal_field_rtc_sda" value="8"></div>
          <div class="form-item"><label class="form-label" for="modal_field_rtc_scl">I2C SCL</label><input type="number" class="form-control" id="modal_field_rtc_scl" value="9"></div>
          <div class="form-item"><label class="form-label" for="modal_field_rtc_int">Ngắt INT</label><input type="number" class="form-control" id="modal_field_rtc_int" value="17"></div>
        `;
      } else if (type === "actuator_pca9685") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_pca_sda">I2C SDA</label><input type="number" class="form-control" id="modal_field_pca_sda" value="8"></div>
          <div class="form-item"><label class="form-label" for="modal_field_pca_scl">I2C SCL</label><input type="number" class="form-control" id="modal_field_pca_scl" value="9"></div>
          <div class="form-item"><label class="form-label" for="modal_field_pca_oe">Chân OE</label><input type="number" class="form-control" id="modal_field_pca_oe" value="-1"></div>
        `;
      } else if (type === "comm_twai") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_twai_tx">TWAI TX</label><input type="number" class="form-control" id="modal_field_twai_tx" value="15"></div>
          <div class="form-item"><label class="form-label" for="modal_field_twai_rx">TWAI RX</label><input type="number" class="form-control" id="modal_field_twai_rx" value="16"></div>
        `;
      } else if (type === "comm_4g") {
        fieldsContainer.innerHTML = `
          <div class="form-item"><label class="form-label" for="modal_field_4g_tx">UART TX</label><input type="number" class="form-control" id="modal_field_4g_tx" value="17"></div>
          <div class="form-item"><label class="form-label" for="modal_field_4g_rx">UART RX</label><input type="number" class="form-control" id="modal_field_4g_rx" value="18"></div>
          <div class="form-item"><label class="form-label" for="modal_field_4g_pwr">PWRKEY</label><input type="number" class="form-control" id="modal_field_4g_pwr" value="2"></div>
        `;
'''
fields_target = '} else if (type === "sensor_dht") {'
app_js = app_js.replace(fields_target, fields_injection.strip('\n') + '\n      ' + fields_target)

names_injection = '''
          sensor_nfc: "RFID/NFC RC522/PN532",
          sensor_rtc: "Đồng hồ RTC DS3231",
          actuator_pca9685: "Mở rộng PWM PCA9685",
'''
names_target = 'sensor_dht: "Nhiệt/Ẩm DHT11/22",'
app_js = app_js.replace(names_target, names_injection.strip('\n') + '\n          ' + names_target)

names_comm_inj = '''
          comm_twai: "Mạng CAN/TWAI Bus",
          comm_4g: "Module 4G LTE",
'''
names_comm_target = 'uart: "UART mở rộng",'
app_js = app_js.replace(names_comm_target, names_comm_inj.strip('\n') + '\n          ' + names_comm_target)

with open('app.js', 'w', encoding='utf-8') as f:
    f.write(app_js)
print("Patch applied to app.js")
