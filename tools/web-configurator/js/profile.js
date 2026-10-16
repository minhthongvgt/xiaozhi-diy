export const S3_PROFILE = { chip: 'esp32s3', board: 'esp32s3-n16r8-custom', board_name: 'ESP32-S3 N16R8 Custom', build_dir: 'build' };

export const P4_BOARDS = [
  { id: 'guition-m3-dev', name: 'Guition JC-ESP32P4-M3-Dev' },
  { id: 'function-ev', name: 'Espressif ESP32-P4-Function-EV-Board' },
  { id: 'waveshare-dev-kit', name: 'Waveshare ESP32-P4-Module-DEV-KIT' },
];

const P4_KEY_PATTERNS = [
  /^CONFIG_OTA_URL$/,
  /^CONFIG_LANGUAGE_/,
  /^CONFIG_FLASH_(DEFAULT|CUSTOM|EXPRESSION|NONE)_ASSETS$/,
  /^CONFIG_CUSTOM_ASSETS_FILE$/,
  /^CONFIG_USE_(DEFAULT|WECHAT|EMOTE)_MESSAGE_STYLE$/,
  /^CONFIG_USE_MULTILINE_CHAT_MESSAGE$/,
  /^CONFIG_CUSTOM_UI_/,
  /^CONFIG_(WAKE_WORD_DISABLED|USE_AFE_WAKE_WORD|USE_CUSTOM_WAKE_WORD|SEND_WAKE_WORD_DATA|WAKE_WORD_DETECTION_IN_LISTENING)$/,
  /^CONFIG_CUSTOM_WAKE_WORD(_DISPLAY|_THRESHOLD)?$/,
  /^CONFIG_(USE_AUDIO_PROCESSOR|USE_DEVICE_AEC|USE_SERVER_AEC|USE_NO_AEC|USE_SOFTWARE_AEC_REFERENCE)$/,
  /^CONFIG_SOUND_/,
  /^CONFIG_(USE_AUDIO_DEBUGGER|AUDIO_DEBUG_UDP_SERVER|RECEIVE_CUSTOM_MESSAGE)$/,
  /^CONFIG_(USE_HOTSPOT_WIFI_PROVISIONING|USE_ESP_BLUFI_WIFI_PROVISIONING)$/,
  /^CONFIG_LOG_DEFAULT_LEVEL_/,
];

export function isP4Profile(profile) {
  return Boolean(profile && profile.chip === 'esp32p4');
}

export function profileValue(profile) {
  return isP4Profile(profile) ? `esp32p4:${profile.board}` : 'esp32s3';
}

export function parseProfileValue(value) {
  const [chip, board] = String(value || '').split(':');
  if (chip === 'esp32p4' && P4_BOARDS.some(b => b.id === board)) return { chip, board };
  return { chip: 'esp32s3' };
}

export function filterLinesForP4(lines) {
  return lines.filter(line => {
    const m = /^(?:# )?(CONFIG_[A-Za-z0-9_]+)/.exec(line);
    return Boolean(m) && P4_KEY_PATTERNS.some(re => re.test(m[1]));
  });
}

export function linesForProfile(lines, profile) {
  return isP4Profile(profile) ? filterLinesForP4(lines) : lines;
}

export function applyProfileToDocument(profile) {
  if (typeof document === 'undefined' || !document.body) return;
  document.body.dataset.chip = isP4Profile(profile) ? 'esp32p4' : 'esp32s3';
}

if (typeof window !== 'undefined') {
  window.buildProfile = window.buildProfile || S3_PROFILE;
}
