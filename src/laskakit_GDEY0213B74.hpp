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
    {
        EPDBus::Begin(settings);
        display.epd2.setBusyCallback(busyCallbackLightSleep, nullptr);
        display.init();
    }

    ~GDEY0213B74()
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
