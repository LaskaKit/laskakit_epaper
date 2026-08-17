#pragma once

#include <Adafruit_GFX.h>
#include "qrcode.h"
#include "laskakit_epaper.hpp"

namespace LaskaKit::Epaper {


template <class T>
class GFX : public Adafruit_GFX {
private:
  T *display;

public:
  static constexpr uint8_t COLOR_BLACK = 0;
  static constexpr uint8_t COLOR_WHITE =
      T::COLORTYPE == ColorType::G4  ? 3  :
      T::COLORTYPE == ColorType::G8  ? 7  :
      T::COLORTYPE == ColorType::G16 ? 15 : 1;

  GFX(T *display) : Adafruit_GFX(T::WIDTH, T::HEIGHT), display(display) {}

  void fillScreen(uint8_t color) { Adafruit_GFX::fillScreen((uint16_t)color); }
  void setTextColor(uint8_t color) { Adafruit_GFX::setTextColor((uint16_t)color); }

  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    int16_t temp;
    switch (this->rotation) {
    case 1: // 90° clockwise
      temp = x;
      x = T::WIDTH - 1 - y;
      y = temp;
      break;
    case 2: // 180°
      x = T::WIDTH - 1 - x;
      y = T::HEIGHT - 1 - y;
      break;
    case 3: // 270° clockwise
      temp = x;
      x = y;
      y = T::HEIGHT - 1 - temp;
      break;
    }
    display->drawPixel(x, y, color);
  }

  void drawQRCodeText(int16_t pos_x, int16_t pos_y, const char *text,
                      uint8_t fg = 0, uint8_t bg = 1,
                      uint8_t scale = 4) {
    static QRCode qrcode;
    uint8_t qrCodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrCodeData, 3, ECC_LOW, text);
    for (int y = 0; y < qrcode.size; y++) {
      for (int x = 0; x < qrcode.size; x++) {
        if (qrcode_getModule(&qrcode, x, y)) {
          this->fillRect(pos_x + x * scale, pos_y + y * scale, scale, scale, fg);
        } else {
          this->fillRect(pos_x + x * scale, pos_y + y * scale, scale, scale, bg);
        }
      }
    }
  }

  void fullUpdate() {
    this->display->fullUpdate();
  }

  void _drawColorSwatch(uint8_t numColors, uint16_t posX, uint16_t swatchWidth = 40) {
      uint16_t swatchPartHeight = this->height() / numColors;
      this->setTextSize(1);
      this->setTextColor(0);
      for (uint8_t i = 0; i < numColors; i++) {
          uint16_t posY = swatchPartHeight * i;
          this->fillRect(posX, posY, swatchWidth, swatchPartHeight, i);
          this->setCursor(posX - 10, posY + swatchPartHeight / 2 - 4);
          this->print(i);
      }
      this->fillRect(posX, 0, 2, this->height(), 0);
      this->fillRect(posX + swatchWidth - 2, 0, 2, this->height(), 0);
      this->fillRect(posX, 0, swatchWidth, 2, 0);
      this->fillRect(posX, this->height() - 2, swatchWidth, 2, 0);
  }

  void drawColorSwatch()
  {
      switch (T::COLORTYPE) {
        case ColorType::G4:
            _drawColorSwatch(4, this->width() - 40);
            break;
        case ColorType::G8:
            _drawColorSwatch(8, this->width() - 40);
            break;
        case ColorType::G16:
            _drawColorSwatch(16, this->width() - 40);
            break;
        case ColorType::C4:
        case ColorType::BWRY:
            _drawColorSwatch(4, this->width() - 40);
            break;
        case ColorType::C6:
            _drawColorSwatch(6, this->width() - 40);
            break;
        case ColorType::BWR:
        case ColorType::RBW:
        case ColorType::BWY:
        case ColorType::YBW:
            _drawColorSwatch(3, this->width() - 40);
            break;
        case ColorType::C7:
            _drawColorSwatch(7, this->width() - 40);
            break;
        case ColorType::BW:
            _drawColorSwatch(2, this->width() - 40);
            break;
      }
  }
};


}; // namespace LaskaKit::Epaper
