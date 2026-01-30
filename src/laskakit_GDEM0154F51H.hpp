#pragma once

#include <GxEPD2_4C.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {
    class GDEM0154F51H {
    public:
        static constexpr size_t WIDTH = 200;
        static constexpr size_t HEIGHT = 200;
        static constexpr ColorType COLORTYPE = ColorType::C4;
        static constexpr const char* NAME = "GDEM0154F51H";
    private:
        GxEPD2_4C<GxEPD2_154c_GDEM0154F51H, GxEPD2_154c_GDEM0154F51H::HEIGHT> display;

    public:
        GDEM0154F51H(const EPDBusSettings& settings)
            : display(GxEPD2_154c_GDEM0154F51H(settings.cs, settings.dc, settings.reset, settings.busy))
        {
            SPI.begin(settings.sck, -1, settings.mosi);
            display.init();
        }

        ~GDEM0154F51H()
        {
            SPI.end();
        }

        void fullUpdate()
        {
            display.display();
        }

        void drawPixel(int16_t x, int16_t y, uint16_t color)
        {
            display.drawPixel(x, y, color);
        }
    };
}
