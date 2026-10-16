#ifndef SOUND_THEME_H
#define SOUND_THEME_H

#include <string_view>

namespace SoundTheme {

std::string_view Startup();
std::string_view Listening();
std::string_view Error();
std::string_view Alert();
std::string_view LowBattery();
std::string_view WifiConfig();
std::string_view Activation();
std::string_view Upgrade();

}

#endif
