#pragma once

#include <Arduino.h>

namespace LaskaKit::Epaper {
enum class ColorType {
  BW,  // black and white
  G4,  // four shades of gray
  G8,  // eight shades of gray
  RBW, // red, black, white
  YBW, // yellow, black, white
  C4,  // four colors (black, white, red, yellow)
  C7,  // seven colors
};

enum class RGB565 : uint16_t {
  BLACK = 0x0000,
  WHITE = 0xFFFF,
  LIGHT_GRAY = 0xAD43,
  LIGHT_GRAY2 = 0xD6BA,
  LIGHT_GRAY3 = 0x8430,
  DARK_GRAY = 0x528A,
  DARK_GRAY2 = 0x2945,
  DARK_GRAY3 = 0x6B4D,
  RED = 0xF800,
  YELLOW = 0xFFE0,
  GREEN = 0x07E0,
  BLUE = 0x001F,
  ORANGE = 0xFBE0,
  PURPLE = 0x7814,
};

constexpr bool operator==(uint16_t lhs, RGB565 rhs) noexcept {
  return lhs == static_cast<uint16_t>(rhs);
}

const char *colorTypeToCStr(ColorType colorType) {
  switch (colorType) {
  case ColorType::BW:
    return "BW";
  case ColorType::G4:
    return "4G";
  case ColorType::G8:
    return "8G";
  case ColorType::RBW:
    return "RBW";
  case ColorType::YBW:
    return "YBW";
  case ColorType::C4:
    return "4C";
  case ColorType::C7:
    return "7C";
  }
  return "";
}


// for GxEPD2 driver
void busyCallbackLightSleep(const void *)
{
    // Enter light sleep for short periods while display is refreshing
    // Wake up after 100ms to check BUSY status again
    esp_sleep_enable_timer_wakeup(100 * 1000);
    esp_light_sleep_start();
}


} // namespace LaskaKit::Epaper
