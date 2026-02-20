#pragma once

#include <Adafruit_GFX.h>
#include "qrcode.h"
#include "laskakit_epaper.hpp"
#include "zdecoder.h"

namespace LaskaKit::Epaper {


template <class T>
class GFX : public Adafruit_GFX {
private:
  T *display;

public:
  GFX(T *display) : Adafruit_GFX(T::WIDTH, T::HEIGHT), display(display) {}

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
                      int16_t fg = 0xFFFF, int16_t bg = 0x0000,
                      uint8_t scale = 4) {
    static QRCode qrcode;
    uint8_t qrCodeData[qrcode_getBufferSize(2)];
    qrcode_initText(&qrcode, qrCodeData, 2, ECC_LOW, text);
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

  void _drawColorSwatch(const uint16_t* colorLut, uint8_t numColors, uint16_t posX, uint16_t swatchWidth = 20) {
      uint16_t swatchPartHeight = this->height() / numColors;
      for (size_t i = 0; i < 4; i++) {
          uint16_t posY = swatchPartHeight * i;
          this->fillRect(posX, posY, swatchWidth, swatchPartHeight, colorLut[i]);
      }
      this->fillRect(posX, 0, 2, this->height(), colorLut[1]);
      this->fillRect(posX + swatchWidth -2, 0, 2, this->height(), colorLut[1]);
      this->fillRect(posX, 0, swatchWidth, 2, colorLut[1]);
      this->fillRect(posX, this->height() - 2, swatchWidth, 2, colorLut[1]);
  }

  void drawColorSwatch()
  {
      switch (T::COLORTYPE) {
          case ColorType::G4:
              _drawColorSwatch(z2GrayscaleToRGB565Lut, 4, this->width() - 20);
              break;
          case ColorType::C4:
          case ColorType::RBW:
          case ColorType::YBW:
              _drawColorSwatch(z2ColorToRGB565Lut, 4, this->width() - 20);
              break;
        case ColorType::BW:
            _drawColorSwatch(z2GrayscaleToRGB565Lut, 2, this->width() - 20);
            break;
        }
  }
};


}; // namespace LaskaKit::Epaper
