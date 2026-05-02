#pragma once

#include <bb_epaper.h>

#include "laskakit_epaper.hpp"
#include "laskakit_bbep.hpp"
#include "epdbus.hpp"


namespace LaskaKit::Epaper {
    class GDEM0154F51H {
    public:
        static constexpr size_t WIDTH = 200;
        static constexpr size_t HEIGHT = 200;
        static constexpr ColorType COLORTYPE = ColorType::C4;
        static constexpr const char* NAME = "GDEM0154F51H";

    public:
        GDEM0154F51H(const EPDBusSettings& settings)
        {}

        bool init()
        {
            epd.setPanelType(EP154YR_200x200);
            epd.initIO(
                settings.dc,
                settings.reset,
                settings.busy,
                settings.cs,
                settings.mosi,
                settings.sck
            );
            epd.setLightSleep(true); // TODO: does it work with AP mode?
            epd.allocBuffer(true);
            return true;
        }

        void fullUpdate()
        {
            epd.writePlane();
            epd.refresh(REFRESH_FULL, true);
            epd.sleep(DEEP_SLEEP);
        }

        void drawPixel(int16_t x, int16_t y, uint16_t color)
        {
            if (color == RGB565::BLACK) {
                epd.drawPixel(x, y, BBEP_BLACK);
            } else if (color == RGB565::WHITE) {
                epd.drawPixel(x, y, BBEP_WHITE);
            } else if (color == RGB565::RED) {
                epd.drawPixel(x, y, BBEP_RED);
            } else if (color == RGB565::YELLOW) {
                epd.drawPixel(x, y, BBEP_YELLOW);
            }
        }
    };
}
