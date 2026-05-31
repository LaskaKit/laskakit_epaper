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
    static constexpr ColorType COLORTYPE = ColorType::BWRY;
    static constexpr const char* NAME = "GDEM042F52";

private:
    GxEPD2_4C<GxEPD2_420c_GDEY0420F51, GxEPD2_420c_GDEY0420F51::HEIGHT> display;

public:
    GDEM042F52(const EPDBusSettings& settings)
        : display(GxEPD2_420c_GDEY0420F51(settings.cs, settings.dc, settings.reset, settings.busy))
    {}

    bool init()
    {
        display.epd2.setBusyCallback([](const void* data) {
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
            case 2:
                display.drawPixel(x, y, GxEPD_RED);
                break;
            case 3:
                display.drawPixel(x, y, GxEPD_YELLOW);
                break;
        }
    }
};

}; // namespace LaskaKit::Epaper
