#include "knob.h"

static const char* TAG = "Knob";

Knob::Knob(gpio_num_t pin_a, gpio_num_t pin_b) : knob_handle_(NULL), pin_a_(pin_a), pin_b_(pin_b) {
    if (!GPIO_IS_VALID_GPIO(pin_a) || !GPIO_IS_VALID_GPIO(pin_b)) {
        ESP_LOGW(TAG, "Invalid encoder pins provided (A:%d, B:%d), skipping init", (int)pin_a, (int)pin_b);
        return;
    }

    knob_config_t config = {
        .default_direction = 0,
        .gpio_encoder_a = static_cast<uint8_t>(pin_a),
        .gpio_encoder_b = static_cast<uint8_t>(pin_b),
    };

    knob_handle_ = iot_knob_create(&config);
    if (knob_handle_ == NULL) {
        ESP_LOGE(TAG, "Failed to create knob instance on pins A:%d B:%d", (int)pin_a, (int)pin_b);
        return;
    }

    esp_err_t err = iot_knob_register_cb(knob_handle_, KNOB_LEFT, knob_callback, this);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register KNOB_LEFT callback: %s", esp_err_to_name(err));
    }

    err = iot_knob_register_cb(knob_handle_, KNOB_RIGHT, knob_callback, this);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register KNOB_RIGHT callback: %s", esp_err_to_name(err));
    }

    ESP_LOGI(TAG, "Knob initialized with pins A:%d B:%d", (int)pin_a, (int)pin_b);
}

Knob::~Knob() {
    if (knob_handle_ != NULL) {
        iot_knob_delete(knob_handle_);
        knob_handle_ = NULL;
    }
}

void Knob::OnRotate(std::function<void(bool)> callback) {
    on_rotate_ = callback;
}

void Knob::knob_callback(void* arg, void* data) {
    Knob* knob = static_cast<Knob*>(data);
    knob_event_t event = iot_knob_get_event(arg);
    
    if (knob->on_rotate_) {
        knob->on_rotate_(event == KNOB_RIGHT);
    }
}