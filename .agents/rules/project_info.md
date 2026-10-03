# BẢN ĐỒ CODEBASE (ĐỌC FILE NÀY THAY VÌ QUÉT THƯ MỤC)

## 0. GIAO THỨC ĐẦU PHIÊN (BẮT BUỘC)
1. Đọc file này trước. KHÔNG chạy `list_dir` đệ quy, KHÔNG grep toàn repo để "làm quen".
2. Muốn biết quan hệ giữa các thành phần: `graphify query "<câu hỏi>"` (đồ thị tại `graphify-out/graph.json`). Chỉ đọc `graphify-out/GRAPH_REPORT.md` khi cần tổng quan kiến trúc.
3. Chỉ mở đúng file cần sửa theo bảng ở mục 3 và 4.
4. Sau khi sửa code: chạy `graphify update .` và cập nhật mục 7 (Nhật ký thay đổi) nếu cấu trúc, ID, pinout hoặc luồng dữ liệu thay đổi.
5. Không quét: `.cache/`, `__pycache__/`, `graphify-out/`, `temp_tree.txt`, `temp_dirs.txt`, `sdkconfig.old`, `dependencies.lock`, `managed_components/`, `build/`.

## 1. Tổng quan
- Dự án: Xiaozhi-ESP32 chatbot, chip ESP32-S3-N16R8 (16MB Octal Flash, 8MB Octal PSRAM), ESP-IDF 6.1.
- Thư mục chính thức: `d:\Code\Antigravity\xiaozhi-v2`. Thư mục chạy thử của người dùng: `c:\xiaozhi` (không nhầm với codebase).
- Không có `.git` cục bộ.
- Hệ thống gồm 3 phần: Firmware C/C++ (`main/`), Web Configurator SPA (`tools/web-configurator/`), Python server (`configurator_server.py`, cổng 8080).

## 2. Luồng dữ liệu Web Configurator
```text
Trình duyệt (index.html + js/*.js, ES Modules)
   ├─ main.js: nạp components/*.html vào các placeholder-panel-*, rồi GUIController.init()
   ├─ guiController.js: syncStateToUI() / syncUIToState() đọc-ghi DOM theo id
   ├─ window.configState (khởi tạo từ DEFAULT_CONFIG trong configState.js)
   ├─ parser.js (SdkconfigParser.parse): text sdkconfig -> state
   ├─ generator.js (SdkconfigGenerator.build): state -> mảng dòng CONFIG_*
   ├─ pinValidator.js: kiểm tra khóa/xung đột GPIO, trả { errors, usage }
   └─ window.apiFetch -> http://localhost:8080/api/*  (file:// tự thêm host)
Python server configurator_server.py + sdkconfig_io.py: đọc/ghi sdkconfig, sdkconfig.defaults, chạy idf.py
```
Quy ước: trạng thái dùng chung phải gắn vào `window` (không có bundler). `configState.js` phải được import đầu tiên trong `main.js`. Các dòng gán `window.*` trong `configState.js` được bọc `typeof window !== 'undefined'` để Node test import được.

## 3. Cây thư mục thực tế (đã xác minh)
```text
xiaozhi-v2/
├── .agents/
│   ├── rules/   rules.md (luật always_on) · IDF.md (chuẩn ESP-IDF) · project_info.md (file này)
│   │            debug-triage.md · graphify.md
│   ├── skills/  agent-build-config · agent-core-memory · agent-hardware-bus  (mỗi thư mục có SKILL.md)
│   └── workflows/graphify.md
├── CMakeLists.txt · Kconfig · idf_component.yml · partitions.csv · partitions/
├── sdkconfig · sdkconfig.defaults · sdkconfig.defaults.esp32s3
├── configurator_server.py        API cục bộ cổng 8080 (51KB)
├── configurator.html             trang chuyển hướng
├── run_configurator.bat · build_windows.bat
├── Danh_Sach_Kconfig_WebUI.md    danh mục Kconfig <-> WebUI
├── README.md · MULTI_SENSOR_IMPLEMENTATION_PLAN.md · docs/ · scripts/
├── main/                         firmware: assets audio boards display drivers led notify protocols utils (+ Kconfig.projbuild, CMakeLists.txt)
├── scripts/                      build.py, build_default_assets.py, gen_lang.py, gpio_validator.py, versions.py, ci/, Image_Converter/, ogg_converter/, p3_tools/, spiffs_assets/, tests/
├── partitions/                   16m.csv (bản dùng) + README.md; partitions.csv ở gốc là bản sao
├── docs/superpowers/plans/       kế hoạch đã thực hiện (4 file .md, ngày 2026-09-25 đến 2026-10-01)
├── graphify-out/                 đồ thị tri thức (graph.json, GRAPH_REPORT.md, manifest.json)
└── tools/
    ├── clean_comments.py         xóa comment khỏi mã nguồn
    ├── test_bugs_regression.old.js   LỖI THỜI: dùng app.js cũ, 10/12 test fail, không dùng
    └── web-configurator/
        ├── index.html            khung trang + sidebar 8 tab + project-modal + placeholder-panel-*
        ├── styles.css
        ├── app.js                BẢN MONOLITH CŨ (2393 dòng). index.html KHÔNG nạp. Chỉ 3 test cũ còn require
        ├── sdkconfig_io.py       logic ghi sdkconfig (có test Python)
        ├── start_configurator.bat · run_all_tests.bat
        ├── js/                   main · configState · guiController · buildController · parser · generator · pinValidator · hardware
        └── components/           panel-assistant · display · audio · network · peripherals · power · system · build
                                  (còn file mồ côi, chưa được index.html nạp: actuators, buttons, camera, flash, led, mcp, motor, sensors, uart, wakeword)
```

## 4. Bảng "muốn sửa X thì mở file Y"
| Việc cần làm | File |
|---|---|
| Thêm/đổi trường cấu hình | `js/configState.js` (DEFAULT_CONFIG) + `js/parser.js` + `js/generator.js` + id trong `components/panel-*.html` + `syncStateToUI`/`syncUIToState` trong `js/guiController.js` |
| Luật xung đột/khóa GPIO | `js/pinValidator.js`, `js/hardware.js` (ESP32S3.LOCKED, MIN, MAX) |
| Tên tab trong modal "chưa lưu" | `GUIController.TAB_NAMES` (đầu `js/guiController.js`) |
| Chọn/mở dự án (modal) | `openProjectModal`, `loadDetectedProjects` (container id `modal-detected-projects-list`) trong `js/guiController.js`; id modal trong `index.html` |
| Build/flash/log IDF | `js/buildController.js` + API `/api/idf/*` trong `configurator_server.py` |
| Đọc/ghi sdkconfig phía server | `configurator_server.py`, `tools/web-configurator/sdkconfig_io.py` |
| Kconfig firmware | `main/Kconfig.projbuild` |

## 5. Bảng panel đang hoạt động (id section = data-target ở sidebar)
`panel-assistant`, `panel-display`, `panel-audio`, `panel-network`, `panel-peripherals`, `panel-power`, `panel-system`, `panel-build`.
`index.html` nạp panel qua `<div id="placeholder-panel-...">`; peripherals dùng `placeholder-panel-peripherals`.

## 6. Kiểm thử (chạy trong `tools/web-configurator/`)
- Tất cả: `run_all_tests.bat` (trạng thái gần nhất: PASS toàn bộ).
- Thành phần: `node test_modules.mjs` (module thật `js/`) · `python test_sdkconfig_io.py` · `node test_dom.js` · `node test_runner.js` · `node test_integration.js` · `node test_kconfig_parity.js`.
- Lưu ý: `test_runner/integration/kconfig_parity` chạy trên `app.js` cũ, không phản ánh lỗi của `js/*.js`; `test_modules.mjs` mới là test của module thật.

## 7. Lỗi đã biết / việc đang dở (cập nhật khi đổi)
- ĐÃ SỬA: `index.html` thiếu `<script type="module" src="js/main.js">`; id `modal-detected-projects-list`; `TAB_NAMES` thiếu panel mới; id input của `panel-system.html`; `window` không tồn tại trong Node.
- ĐANG CHỜ (kế hoạch `comprehensive_debug_plan.md`):
  1. `components/panel-peripherals.html`: nhiều input thiếu hoặc lệch id so với `guiController.js` (ví dụ `dc_motor_enable` vs `motor_dc_enable`, `dht_enable` vs `sensor_dht`, thiếu id cho touch/vol/relay/haptic/servo/hcsr04/pir/vibration).
  2. `guiController.js` chưa đồng bộ `s.network.cellular_*`, `s.network.uart_ext_*`, `s.network.mcp_enable` với `panel-network.html`.
  3. `pinValidator.js` chưa đăng ký chân 4G và UART mở rộng.
  4. `syncStateToUI` chưa map `s.assistant.*`.
  5. Xử lý lệch giữa `s.mcp.enable`/`mcp_enable` và `s.network.mcp_enable`.
- Dọn dẹp tùy chọn: xóa hoặc chuyển `app.js`, `test_bugs_regression.old.js`, và các panel mồ côi sau khi di trú test sang module thật.

## 8. Quy ước bắt buộc (tóm tắt, chi tiết ở `rules.md` và `IDF.md`)
- Không sinh comment trong code, mọi ngôn ngữ.
- GPIO: `GPIO_IS_VALID_GPIO()`, không dùng `GPIO_NUM_NC`/`GPIO_NUM_MAX`; chân không dùng là `-1`.
- C: `typedef struct name_t { ... } name_t;`. C++: không exception, `static_cast<gpio_num_t>`.
- Touch driver: `CONFIG_TOUCH_SUPPRESS_DEPRECATE_WARN=y` và bọc pragma `-Wcpp`.
- Audio I2S: `I2S_STD_SLOT_BOTH`, DMA 8x480, queue 4, volume 0-100 và headroom 0.85.
- Static theo Kconfig: bọc `#if` và `__attribute__((unused))`.
- Chỉ sửa đúng tệp được yêu cầu; có bằng chứng (chạy lệnh) trước khi báo hoàn tất.

## 9. Nhật ký thay đổi cấu trúc
- 2026-10-03: viết lại bản đồ codebase; sửa 5 lỗi Web UI (mục 7); thêm quy tắc không-comment (rules 12-13, IDF.md mục 10, 3 SKILL, debug-triage); chuẩn hóa README, IDF.md mục 7, partitions/README.md, agent-build-config SKILL theo codebase thực tế.

## 10. API Python server (cổng 8080) và phân vùng flash
- GET: `/api/project`, `/api/project/scan-all`, `/api/config`, `/api/idf/status`, `/api/idf/scan-all`, `/api/idf/ports`, `/api/idf/logs?offset=`.
- POST: `/api/save`, `/api/save-tab`, `/api/project/set-root`, `/api/project/reset-root`, `/api/idf/set-path`, `/api/idf/reset-path`, `/api/idf/set-target`, `/api/idf/build`, `/api/idf/flash`, `/api/idf/build-flash`, `/api/idf/clean`, `/api/idf/cancel`, `/api/idf/clear-logs`.
- Cấu hình lưu cạnh Web UI: `.project_config.json`, `.idf_config.json`. Thứ tự xác định thư mục dự án: cấu hình đã lưu, `--project`, biến môi trường (`XIAOZHI_PROJECT_ROOT`, `ESP_PROJECT_ROOT`, `PROJECT_ROOT`), vị trí script, CWD, quét ổ đĩa.
- Phân vùng (`partitions/16m.csv`): nvs 0x9000 64KB, otadata 0x19000 8KB, phy_init 0x1B000 4KB, app0 0x20000 4.5MB, app1 0x4A0000 4.5MB, `model` 0x920000 6.9MB (spiffs). Không có `storage`, không có `partitions/v2/`. Tên `assets` chỉ là NVS namespace.

## 11. Chỉ mục tài liệu .md (mục đích, trạng thái)
| File | Mục đích | Trạng thái |
|---|---|---|
| `.agents/rules/rules.md` | luật always_on, nạp IDF.md và project_info.md | chuẩn |
| `.agents/rules/IDF.md` | chuẩn mã ESP-IDF 6.1 (mục 1-10) | chuẩn, mục 7 đã sửa phân vùng |
| `.agents/rules/project_info.md` | bản đồ codebase (file này) | chuẩn |
| `.agents/rules/debug-triage.md` | phân loại lỗi → 3 SKILL | chuẩn |
| `.agents/rules/graphify.md`, `.agents/workflows/graphify.md` | dùng graphify | chuẩn |
| `.agents/skills/*/SKILL.md` (3) | đặc vụ core-memory, build-config, hardware-bus | chuẩn, có quy chuẩn không-comment |
| `README.md` | hướng dẫn người dùng, pinout, quy trình build | đã sửa tab, phân vùng, test. Mô tả chi tiết nhóm chức năng vẫn theo kiến trúc cũ (nút "+ Thêm ...") |
| `partitions/README.md` | quy ước phân vùng `model` | chuẩn |
| `Danh_Sach_Kconfig_WebUI.md` | bảng Kconfig ↔ WebUI (48KB, tạo tự động) | tham chiếu, không sửa tay |
| `MULTI_SENSOR_IMPLEMENTATION_PLAN.md` | kế hoạch đa cảm biến (`EXTRA_GPIOS`) | kế hoạch thiết kế |
| `docs/superpowers/plans/*.md` | kế hoạch đã thực hiện | lịch sử |
| `audit_report_full.md` | báo cáo quét tự động cũ (214 file) | lỗi thời, có thể xoá |
| `graphify-out/GRAPH_REPORT.md` | tổng quan đồ thị (tự sinh) | không sửa tay |
