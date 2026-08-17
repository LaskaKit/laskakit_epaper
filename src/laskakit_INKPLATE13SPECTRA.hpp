#pragma once

#include <Inkplate.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"


namespace LaskaKit::Epaper {
    // Soldered Inkplate 13SPECTRA, 13.3" E Ink Spectra 6 panel, 1600x1200.
    // Wraps the vendor InkplateLibrary (e-radionicacom/InkplateLibrary) instead
    // of talking to the panel directly, since it already owns the correct
    // init/waveform sequence for this board.
    class Inkplate13Spectra {
    public:
        static constexpr size_t WIDTH  = 1600;
        static constexpr size_t HEIGHT = 1200;
        static constexpr ColorType COLORTYPE = ColorType::C6;
        static constexpr const char* NAME = "INKPLATE13SPECTRA";

    private:
        Inkplate display;

    public:
        Inkplate13Spectra(const EPDBusSettings& settings)
        {}

        bool init()
        {
            display.begin();
            display.clearDisplay();
            display.display();
            return true;
        }

        void fullUpdate()
        {
            display.display();
        }

        double readBattery()
        {
            return display.readBattery();
        }

        // maps the generic GFX palette index to the panel's INKPLATE_* colors,
        // same constants/offsets spectra.ino uses (INKPLATE_GREEN-1, INKPLATE_BLUE-1)
        void drawPixel(int16_t x, int16_t y, uint8_t color)
        {
            uint16_t panelColor;
            switch (color) {
                case 0:
                    panelColor = INKPLATE_BLACK;
                    break;
                case 1:
                    panelColor = INKPLATE_WHITE;
                    break;
                case 2:
                    panelColor = INKPLATE_RED;
                    break;
                case 3:
                    panelColor = INKPLATE_YELLOW;
                    break;
                case 4:
                    panelColor = INKPLATE_GREEN - 1;
                    break;
                case 5:
                    panelColor = INKPLATE_BLUE - 1;
                    break;
                default:
                    return;
            }
            display.drawPixel(x, y, panelColor);
        }
    };
}
