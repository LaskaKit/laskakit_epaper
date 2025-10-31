#pragma once

#include "laskakit_epaper.hpp"

namespace LaskaKit::Epaper
{
    class None : public Display
    {
    public:
        static constexpr uint WIDTH = 0;
        static constexpr uint HEIGHT = 0;

        None(){}
        void fullUpdate(){}
        void drawPixel(int x, int y, uint8_t color){};
    };
}