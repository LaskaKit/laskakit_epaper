#pragma once

#include <GxEPD2.h>
#include <GxEPD2_BW.h>

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {

class GDEY0213B74 {
public:
    static constexpr size_t WIDTH = 122;
    static constexpr size_t HEIGHT = 250;
    static constexpr ColorType COLORTYPE = ColorType::BW;
    static constexpr const char* NAME = "GDEY0213B74";

private:
    GxEPD2_BW<GxEPD2_213_GDEY0213B74, GxEPD2_213_GDEY0213B74::HEIGHT> display;

public:
    GDEY0213B74(const EPDBusSettings& settings)
        : display(GxEPD2_213_GDEY0213B74(settings.cs, settings.dc, settings.reset, settings.busy))
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

}; // namespace LaskaKit::Epaper
