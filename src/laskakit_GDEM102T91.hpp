#pragma once

#include <GxEPD2_BW.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {
    class GDEM102T91 {
    public:
        static constexpr size_t WIDTH = 960;
        static constexpr size_t HEIGHT = 640;
        static constexpr ColorType COLORTYPE = ColorType::BW;
        static constexpr const char* NAME = "GDEM102T91";
    private:
        GxEPD2_BW<GxEPD2_1020_GDEM102T91, GxEPD2_1020_GDEM102T91::HEIGHT> display;

    public:
        GDEM102T91(const EPDBusSettings& settings)
            : display(GxEPD2_1020_GDEM102T91(settings.cs, settings.dc, settings.reset, settings.busy))
        {
            EPDBus::Begin(settings);
            display.epd2.setBusyCallback(busyCallbackLightSleep, nullptr);
            display.init();
        }

        ~GDEM102T91()
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
