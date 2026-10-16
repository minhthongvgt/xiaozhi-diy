#include "sound_theme.h"

#include "assets/lang_config.h"
#include "sound_theme_map.h"

namespace SoundTheme {

std::string_view Startup() { return SOUND_THEME_STARTUP; }
std::string_view Listening() { return SOUND_THEME_LISTENING; }
std::string_view Error() { return SOUND_THEME_ERROR; }
std::string_view Alert() { return SOUND_THEME_ALERT; }
std::string_view LowBattery() { return SOUND_THEME_LOW_BATTERY; }
std::string_view WifiConfig() { return SOUND_THEME_WIFI_CONFIG; }
std::string_view Activation() { return SOUND_THEME_ACTIVATION; }
std::string_view Upgrade() { return SOUND_THEME_UPGRADE; }

}
