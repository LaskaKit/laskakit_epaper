#pragma once

#include <GxEPD2.h>
#include <GxEPD2_BW.h>

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {

class GDEY029T71H {
public:
    static constexpr size_t WIDTH = 168;
    static constexpr size_t HEIGHT = 384;
    static constexpr ColorType COLORTYPE = ColorType::BW;
    static constexpr const char* NAME = "GDEY029T71H";

private:
    GxEPD2_BW<GxEPD2_290_GDEY029T71H, GxEPD2_290_GDEY029T71H::HEIGHT> display;

public:
    GDEY029T71H(const EPDBusSettings& settings)
        : display(GxEPD2_290_GDEY029T71H(settings.cs, settings.dc, settings.reset, settings.busy))
    {
        EPDBus::Begin(settings);
        display.init();
    }

    ~GDEY029T71H()
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
