#pragma once

#include <GxEPD2_4C.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {
    class GDEY029F51H {
    public:
        static constexpr size_t WIDTH = 168;
        static constexpr size_t HEIGHT = 384;
        static constexpr ColorType COLORTYPE = ColorType::C4;
        static constexpr const char* NAME = "GDEY029F51H";
    private:
        GxEPD2_4C<GxEPD2_290c_GDEY029F51H, GxEPD2_290c_GDEY029F51H::HEIGHT> display;

    public:
        GDEY029F51H(const EPDBusSettings& settings)
            : display(GxEPD2_290c_GDEY029F51H(settings.cs, settings.dc, settings.reset, settings.busy))
        {
            EPDBus::Begin(settings);
            display.epd2.setBusyCallback(busyCallbackLightSleep, nullptr);
            display.init();
        }

        ~GDEY029F51H()
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
