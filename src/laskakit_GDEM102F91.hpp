#pragma once

#include <GxEPD2_4C.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {
    class GDEM102F91 {
    public:
        static constexpr size_t WIDTH = 960;
        static constexpr size_t HEIGHT = 640;
        static constexpr ColorType COLORTYPE = ColorType::C4;
        static constexpr const char* NAME = "GDEM102F91";
    private:
        GxEPD2_4C<GxEPD2_1160c_GDEY116F51, GxEPD2_1160c_GDEY116F51::HEIGHT> display;

    public:
        GDEM102F91(const EPDBusSettings& settings)
            : display(GxEPD2_1160c_GDEY116F51(settings.cs, settings.dc, settings.reset, settings.busy))
        {
            SPI.begin(settings.sck, -1, settings.mosi);
            display.init();
        }

        ~GDEM102F91()
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
