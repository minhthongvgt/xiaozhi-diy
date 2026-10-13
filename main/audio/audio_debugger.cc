#include "audio_debugger.h"

#include "sdkconfig.h"

#if CONFIG_USE_AUDIO_DEBUGGER
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <esp_log.h>
#include <string>
#include <unistd.h>
#include <cstdlib>
#endif

#define TAG "AudioDebugger"

AudioDebugger::AudioDebugger() {
#if CONFIG_USE_AUDIO_DEBUGGER
    udp_sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd_ >= 0) {
        std::string server_addr = CONFIG_AUDIO_DEBUG_UDP_SERVER;
        size_t colon_pos = server_addr.find(':');

        if (colon_pos != std::string::npos) {
            std::string ip = server_addr.substr(0, colon_pos);
            const char* port_text = server_addr.c_str() + colon_pos + 1;
            char* port_end = nullptr;
            long port = std::strtol(port_text, &port_end, 10);
            if (port_end == port_text || port <= 0 || port > 65535) {
                ESP_LOGE(TAG, "Invalid audio debug server port in %s", CONFIG_AUDIO_DEBUG_UDP_SERVER);
                close(udp_sockfd_);
                udp_sockfd_ = -1;
                return;
            }

            memset(&udp_server_addr_, 0, sizeof(udp_server_addr_));
            udp_server_addr_.sin_family = AF_INET;
            udp_server_addr_.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &udp_server_addr_.sin_addr);

            ESP_LOGI(TAG, "Initialized server address: %s", CONFIG_AUDIO_DEBUG_UDP_SERVER);
        } else {
            ESP_LOGW(TAG, "Invalid server address: %s, should be IP:PORT", CONFIG_AUDIO_DEBUG_UDP_SERVER);
            close(udp_sockfd_);
            udp_sockfd_ = -1;
        }
    } else {
        ESP_LOGW(TAG, "Failed to create UDP socket: %d", errno);
    }
#endif
}

AudioDebugger::~AudioDebugger() {
#if CONFIG_USE_AUDIO_DEBUGGER
    if (udp_sockfd_ >= 0) {
        close(udp_sockfd_);
        ESP_LOGI(TAG, "Closed UDP socket");
    }
#endif
}

void AudioDebugger::Feed(const std::vector<int16_t>& data) {
#if CONFIG_USE_AUDIO_DEBUGGER
    if (udp_sockfd_ >= 0) {
        ssize_t sent = sendto(udp_sockfd_, data.data(), data.size() * sizeof(int16_t), 0,
            reinterpret_cast<struct sockaddr*>(&udp_server_addr_), sizeof(udp_server_addr_));
        if (sent < 0) {
            ESP_LOGW(TAG, "Failed to send audio data to %s: %d", CONFIG_AUDIO_DEBUG_UDP_SERVER, errno);
        } else {
            ESP_LOGD(TAG, "Sent %d bytes audio data to %s", sent, CONFIG_AUDIO_DEBUG_UDP_SERVER);
        }
    }
#endif
}
