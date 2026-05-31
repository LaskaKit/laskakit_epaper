#pragma once

#include <GxEPD2_BW.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {
    class GDEY116T91 {
    public:
        static constexpr size_t WIDTH = 960;
        static constexpr size_t HEIGHT = 640;
        static constexpr ColorType COLORTYPE = ColorType::BW;
        static constexpr const char* NAME = "GDEY116T91";
    private:
        GxEPD2_BW<GxEPD2_1160_T91, GxEPD2_1160_T91::HEIGHT> display;

    public:
        GDEY116T91(const EPDBusSettings& settings)
            : display(GxEPD2_1160_T91(settings.cs, settings.dc, settings.reset, settings.busy))
        {}

        bool init()
        {
            display.epd2.setBusyCallback([](const void*) {
                EPDBus::WaitBusyLow();
            }, nullptr);
            display.init();
            return true;
        }

        void fullUpdate()
        {
            display.display();
        }

        void drawPixel(int16_t x, int16_t y, uint8_t color)
        {
            switch (color) {
                case 0:
                    display.drawPixel(x, y, GxEPD_BLACK);
                    break;
                case 1:
                    display.drawPixel(x, y, GxEPD_WHITE);
                    break;
            }
        }
    };
}
