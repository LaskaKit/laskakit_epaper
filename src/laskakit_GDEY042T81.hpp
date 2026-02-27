#pragma once

#include <GxEPD2_BW.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {
    class GDEY042T81 {
    public:
        static constexpr size_t WIDTH = 400;
        static constexpr size_t HEIGHT = 300;
        static constexpr ColorType COLORTYPE = ColorType::BW;
        static constexpr const char* NAME = "GDEY042T81";
    private:
        GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display;

    public:
        GDEY042T81(const EPDBusSettings& settings)
            : display(GxEPD2_420_GDEY042T81(settings.cs, settings.dc, settings.reset, settings.busy))
        {
            EPDBus::Begin(settings);
            display.init();
        }

        ~GDEY042T81()
        {
            EPDBus::End();
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
