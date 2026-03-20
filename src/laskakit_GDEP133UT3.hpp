#pragma once

#include <FastEPD.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

namespace LaskaKit::Epaper {


    class GDEP133UT3 {
    public:
        static constexpr size_t WIDTH = 1600;
        static constexpr size_t HEIGHT = 1200;
        static constexpr ColorType COLORTYPE = ColorType::G8;
        static constexpr const char* NAME = "GDEP133UT3";

    private:
        bool initialized = false;
        static FASTEPD epaper;

        void init()
        {
            epaper.initPanel(BB_PANEL_EPDIY_V7);
            epaper.setPanelSize(1600, 1200);
            epaper.setMode(BB_MODE_4BPP);
            this->initialized = true;
        }
    public:
        GDEP133UT3(const EPDBusSettings& settings) {}

        void fullUpdate()
        {
            if (!initialized) {
                init();
            }
            epaper.fullUpdate();
        }
        void drawPixel(int16_t x, int16_t y, uint16_t color)
        {
            if (!initialized) {
                init();
            }

            uint8_t col = 0xF;
            if (color == RGB565::WHITE) {
                col = 0xF;
            }
            if (color == RGB565::BLACK) {
                col = 0x0;
            }
            if (color == RGB565::LIGHT_GRAY) {
                col = 0xB;
            }
            if (color == RGB565::LIGHT_GRAY2) {
                col = 0x9;
            }
            if (color == RGB565::LIGHT_GRAY3) {
                col = 0x8;
            }
            if (color == RGB565::DARK_GRAY) {
                col = 0x6;
            }
            if (color == RGB565::DARK_GRAY2) {
                col = 0x4;
            }
            if (color == RGB565::DARK_GRAY3) {
                col = 0x2;
            }
            epaper.drawPixel(x, y, col);
        }
    };

    FASTEPD GDEP133UT3::epaper;

} // LaskaKit::Epaper
