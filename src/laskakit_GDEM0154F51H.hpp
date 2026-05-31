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
        static constexpr ColorType COLORTYPE = ColorType::BWRY;
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

        void drawPixel(int16_t x, int16_t y, uint8_t color)
        {
            switch (color) {
                case 0:
                    epd.drawPixel(x, y, BBEP_BLACK);
                    break;
                case 1:
                    epd.drawPixel(x, y, BBEP_WHITE);
                    break;
                case 2:
                    epd.drawPixel(x, y, BBEP_RED);
                    break;
                case 3:
                    epd.drawPixel(x, y, BBEP_YELLOW);
                    break;
            }
        }
    };
}
