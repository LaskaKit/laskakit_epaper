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
  C6,  // six colors (black, white, red, yellow, green, blue)
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
  case ColorType::YBW:
  case ColorType::BWY:
    return "3C";
  case ColorType::C4:
  case ColorType::BWRY:
    return "4C";
  case ColorType::C6:  // reports as 7C, zivyobraz has no dedicated 6-color code
  case ColorType::C7:
    return "7C";
  }
  return "";
}

} // namespace LaskaKit::Epaper
