#pragma once

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"

namespace LaskaKit::Epaper
{
    class None
    {
    public:
        static constexpr uint WIDTH = 0;
        static constexpr uint HEIGHT = 0;
        static constexpr ColorType COLORTYPE = ColorType::BW;
        static constexpr const char* NAME = "NONE";

        None(const EPDBusSettings& settings){}
        bool init(){return true;}
        void fullUpdate(){}
        void drawPixel(int x, int y, uint8_t color){};
    };
}
