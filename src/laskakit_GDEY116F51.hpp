#pragma once

#include <GxEPD2_4C.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {
    class GDEY116F51 {
    public:
        static constexpr size_t WIDTH = 960;
        static constexpr size_t HEIGHT = 640;
        static constexpr ColorType COLORTYPE = ColorType::BWRY;
        static constexpr const char* NAME = "GDEY116F51";
    private:
        GxEPD2_4C<GxEPD2_1160c_GDEY116F51, GxEPD2_1160c_GDEY116F51::HEIGHT> display;

    public:
        GDEY116F51(const EPDBusSettings& settings)
            : display(GxEPD2_1160c_GDEY116F51(settings.cs, settings.dc, settings.reset, settings.busy))
        {}

        bool init()
        {
            display.epd2.setBusyCallback([](const void*) {
                EPDBus::BusyWaitInv();
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
                case 2:
                    display.drawPixel(x, y, GxEPD_RED);
                    break;
                case 3:
                    display.drawPixel(x, y, GxEPD_YELLOW);
                    break;
            }
        }
    };
}
