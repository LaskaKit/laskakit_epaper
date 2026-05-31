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
  G16,
  BWR,
  BWY,
  BWRY,
};

const char *colorTypeToCStr(ColorType colorType) {
  switch (colorType) {
  case ColorType::BW:
    return "BW";
  case ColorType::G4:
    return "4G";
  case ColorType::G8:
  case ColorType::G16:  // so sad G16 is not supported by zivyobraz
    return "8G";
  case ColorType::RBW:
  case ColorType::BWR:
    return "RBW";
  case ColorType::YBW:
  case ColorType::BWY:
    return "YBW";
  case ColorType::C4:
  case ColorType::BWRY:
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
