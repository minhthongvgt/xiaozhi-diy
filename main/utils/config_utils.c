#include "config_utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int parse_extra_gpios(const char* config_str, extra_gpio_config_t* out_configs, int max_configs) {
    if (!config_str || strlen(config_str) == 0) return 0;
    char* str_copy = strdup(config_str);
    if (!str_copy) return 0;
    int count = 0;
    char* saveptr1;
    char* token = strtok_r(str_copy, ",", &saveptr1);
    while (token != NULL && count < max_configs) {
        while (*token == ' ') token++;
        if (strlen(token) > 0) {
            out_configs[count].pin1 = (gpio_num_t)-1;
            out_configs[count].pin2 = (gpio_num_t)-1;
            out_configs[count].label[0] = '\0';
            char* label_sep = strchr(token, '|');
            if (label_sep) {
                *label_sep = '\0'; 
                strncpy(out_configs[count].label, label_sep + 1, sizeof(out_configs[count].label) - 1);
                out_configs[count].label[sizeof(out_configs[count].label) - 1] = '\0';
            }
            char* colon_sep = strchr(token, ':');
            if (colon_sep) {
                *colon_sep = '\0';
                out_configs[count].pin1 = (gpio_num_t)atoi(token);
                out_configs[count].pin2 = (gpio_num_t)atoi(colon_sep + 1);
            } else {
                out_configs[count].pin1 = (gpio_num_t)atoi(token);
            }
            count++;
        }
        token = strtok_r(NULL, ",", &saveptr1);
    }
    free(str_copy);
    return count;
}
