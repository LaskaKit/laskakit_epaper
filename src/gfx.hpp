#pragma once

#include "qrcode.h"
#include <Adafruit_GFX.h>

namespace LaskaKit::Epaper {

template <class T>
class GFX : public Adafruit_GFX {
private:
  T *display;

public:
  GFX(T *display) : Adafruit_GFX(T::WIDTH, T::HEIGHT), display(display) {}

  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    int16_t temp;
    switch (rotation) {
    case 1: // 90° clockwise
      temp = x;
      x = WIDTH - 1 - y;
      y = temp;
      break;
    case 2: // 180°
      x = WIDTH - 1 - x;
      y = HEIGHT - 1 - y;
      break;
    case 3: // 270° clockwise
      temp = x;
      x = y;
      y = HEIGHT - 1 - temp;
      break;
    }
    display->drawPixel(x, y, color);
  }

  void drawQRCodeText(int16_t pos_x, int16_t pos_y, const char *text,
                      int16_t fg = 0xFFFF, int16_t bg = 0x0000,
                      uint8_t scale = 4) {
    static QRCode qrcode;
    uint8_t qrCodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrCodeData, 3, ECC_LOW, text);

    for (int y = 0; y < qrcode.size; y++) {
      for (int x = 0; x < qrcode.size; x++) {
        if (qrcode_getModule(&qrcode, x, y)) {
          fillRect(pos_x + x * scale, pos_y + y * scale, scale, scale, fg);
        } else {
          fillRect(pos_x + x * scale, pos_y + y * scale, scale, scale, bg);
        }
      }
    }
  }
};

}; // namespace LaskaKit::Epaper