#pragma once

#include <GxEPD2.h>
#include <GxEPD2_4C.h>

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {

class GDEM042F52 {
public:
    static constexpr size_t WIDTH = 400;
    static constexpr size_t HEIGHT = 300;
    static constexpr ColorType COLORTYPE = ColorType::C4;
    static constexpr const char* NAME = "GDEM042F52";

private:
    GxEPD2_4C<GxEPD2_420c_GDEY0420F51, GxEPD2_420c_GDEY0420F51::HEIGHT> display;

public:
    GDEM042F52(const EPDBusSettings& settings)
        : display(GxEPD2_420c_GDEY0420F51(settings.cs, settings.dc, settings.reset, settings.busy))
    {
        EPDBus::Begin(settings);
        display.epd2.setBusyCallback(busyCallbackLightSleep, nullptr);
        display.init();
    }

    ~GDEM042F52()
    {
        EPDBus::End();
    }

    void fullUpdate()
    {
        display.display();
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        if (color == RGB565::WHITE) {
            display.drawPixel(x, y, GxEPD_WHITE);
            return;
        }
        if (color == RGB565::BLACK) {
            display.drawPixel(x, y, GxEPD_BLACK);
            return;
        }
        if (color == RGB565::RED) {
            display.drawPixel(x, y, GxEPD_RED);
            return;
        }
        if (color == RGB565::YELLOW) {
            display.drawPixel(x, y, GxEPD_YELLOW);
            return;
        }
    }
};

}; // namespace LaskaKit::Epaper
