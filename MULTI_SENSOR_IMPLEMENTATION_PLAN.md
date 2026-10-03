# Kế Hoạch Triển Khai: Hỗ trợ Đa Cảm Biến (Multi-Sensor) Động trên Firmware ESP32

## Bối Cảnh
Hiện tại, giao diện Web Configurator đã cho phép người dùng thêm vô số các cảm biến phụ trợ và gán nhãn tùy ý (VD: `SDA:SCL|Nhãn` hoặc `GPIO|Nhãn`). Dữ liệu này được lưu thành công vào file `sdkconfig.defaults` qua các Macro như `CONFIG_CUSTOM_SENSOR_DHT_EXTRA_GPIOS`.
Tuy nhiên, kiến trúc Firmware hiện tại ở lớp `sensor_manager.c` chỉ hỗ trợ khởi tạo và lưu trữ **duy nhất một (01) cảm biến (Singleton)** cho mỗi loại cảm biến.

Kế hoạch này cung cấp từng bước chi tiết (step-by-step) để đập bỏ giới hạn Singleton trong `sensor_manager.c`, cho phép khởi tạo hàng loạt (Mảng - Array) cảm biến cùng lúc.

---

## 1. Mở Rộng Kconfig.projbuild (Bắt Buộc Trước Tiên)
**Vấn đề:** Trình biên dịch (CMake) sẽ bỏ qua các tham số cấu hình trong `sdkconfig.defaults` nếu chúng chưa được định nghĩa trong `Kconfig.projbuild`.
- **Nhiệm vụ:** Mở file `main/Kconfig.projbuild` và thêm tất cả các chuỗi `EXTRA_GPIOS` còn thiếu:
  - `CONFIG_CUSTOM_SENSOR_TEMP_EXTRA_GPIOS` (cho AHT20/SHT3x)
  - `CONFIG_CUSTOM_SENSOR_GAS_EXTRA_GPIOS` (cho SCD4x)
  - `CONFIG_CUSTOM_SENSOR_BMP280_EXTRA_GPIOS`
  - `CONFIG_CUSTOM_SENSOR_BH1750_EXTRA_GPIOS`
  - `CONFIG_CUSTOM_SENSOR_LDR_EXTRA_CHS` (Kênh ADC cho LDR)

## 2. Thiết Kế Cấu Trúc Quản Lý Đa Cảm Biến (Mảng & Nhãn)
**Vấn đề:** Thay vì lưu 1 biến con trỏ (Ví dụ `static aht20_handle_t s_aht20_dev;`), ta cần lưu thành danh sách (List/Array).
- **Nhiệm vụ:** Trong file `main/drivers/sensor/sensor_manager.c`, chuyển đổi cấu trúc khai báo biến:
  ```c
  #define MAX_EXTRA_SENSORS 5

  // Cấu trúc Wrapper quản lý chung từng cảm biến kèm nhãn
  typedef struct {
      aht20_handle_t handle;
      char label[32];
  } aht20_instance_t;

  static aht20_instance_t s_aht20_instances[MAX_EXTRA_SENSORS];
  static int s_num_aht20_instances = 0;
  
  // Áp dụng tương tự cho DHT, BMP280, BH1750...
  ```

## 3. Khắc Phục Giới Hạn Phần Cứng I2C (Quan Trọng Nhất)
**Vấn đề kỹ thuật:** ESP32-S3 chỉ có 2 bộ điều khiển I2C phần cứng (I2C0 và I2C1). Nếu người dùng thêm 4 cảm biến I2C (AHT20) trên 4 cặp chân SDA/SCL khác nhau, phần cứng sẽ bị thiếu tài nguyên.
- **Giải pháp 1 (Soft I2C/Bit-banging):** Sử dụng thư viện I2C bằng phần mềm.
- **Giải pháp 2 (Re-configuration):** Tái sử dụng I2C0. Trước khi đọc 1 cảm biến phụ, Firmware sẽ gọi hàm hủy I2C0 và cài đặt lại I2C0 với cặp chân `SDA/SCL` của cảm biến đó.
- **Nhiệm vụ:** Cập nhật hàm `sensor_manager_init()` để xử lý việc khởi tạo các bus I2C động, hoặc chuẩn bị sẵn hàm chuyển đổi I2C Port.

## 4. Cập Nhật Vòng Lặp Khởi Tạo (Init Loop)
**Nhiệm vụ:** Trong hàm `sensor_manager_init(void)` (file `sensor_manager.c`):
- Sử dụng hàm `parse_extra_gpios()` (từ `config_utils.h`) để bóc tách chuỗi cấu hình.
- Viết vòng lặp `for` chạy qua các chân đã bóc tách.
- Nếu là cảm biến DHT (1-Wire): Rất dễ, chỉ cần lưu mảng `gpio_num_t` kèm nhãn.
- Nếu là cảm biến I2C (AHT20, BMP280...): Tiến hành cấp phát (allocate) thiết bị trên bus I2C tương ứng và lưu `handle` vào mảng.

```c
#if defined(CONFIG_CUSTOM_SENSOR_DHT_EXTRA_GPIOS)
    extra_gpio_config_t dht_configs[MAX_EXTRA_SENSORS];
    int count = parse_extra_gpios(CONFIG_CUSTOM_SENSOR_DHT_EXTRA_GPIOS, dht_configs, MAX_EXTRA_SENSORS);
    for (int i = 0; i < count; i++) {
        // Lưu chân và nhãn vào cấu trúc quản lý
        s_dht_instances[s_num_dht_instances].pin = dht_configs[i].pin1;
        strncpy(s_dht_instances[s_num_dht_instances].label, dht_configs[i].label, 32);
        s_num_dht_instances++;
    }
#endif
```

## 5. Cập Nhật Hàm Đọc Dữ Liệu (Read API) và MQTT/WebSocket
**Vấn đề:** Hiện tại ứng dụng định kỳ gọi hàm `sensor_manager_read_all()` hoặc các hàm lấy thông số để gửi lên App/Web.
- **Nhiệm vụ:** 
  - Đổi các API như `aht20_read(...)` thành vòng lặp đọc toàn bộ mảng `s_aht20_instances`.
  - Cập nhật luồng cấu trúc JSON (trong `device_state_machine.cc` hoặc `mqtt_protocol.cc`) để payload gửi lên mạng có dạng mảng đối tượng:
    ```json
    "dht": [
       {"label": "Phòng Khách", "temp": 28.5, "hum": 60.2},
       {"label": "Phòng Ngủ", "temp": 25.0, "hum": 55.0}
    ]
    ```

## Tóm Lược Khối Lượng Công Việc Dự Kiến:
- **Thời gian ước tính:** ~3 đến 4 phiên làm việc (Prompts).
- **Tệp tin bị ảnh hưởng:**
  - `main/Kconfig.projbuild` (Thêm biến)
  - `main/drivers/sensor/sensor_manager.c` (Sửa kiến trúc mảng)
  - `main/boards/common/bus_manager.c` (Quản lý đa bus I2C)
  - `main/device_state_machine.cc` (Tái cấu trúc luồng dữ liệu)
  - `main/protocols/mqtt_protocol.cc` (Cấu trúc lại JSON)

*Lưu file kế hoạch này lại. Khi tài nguyên API (Token) của bạn được khôi phục, chỉ cần ra lệnh **"Hãy thực hiện theo file MULTI_SENSOR_IMPLEMENTATION_PLAN.md"** để tôi bắt đầu ngay vào việc lập trình.*
