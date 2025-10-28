#pragma once

#include "laskakit_epaper.hpp"

namespace LaskaKit::Epaper
{
    class None : public Display
    {
        static constexpr uint WIDTH = 0;
        static constexpr uint HEIGHT = 0;

    public:
        None(){}
        uint width(){ return this->WIDTH; }
        uint height(){ return this->HEIGHT; }
        void fullUpdate(){}
        void drawPixel(int x, int y, uint8_t color){};
    };
}