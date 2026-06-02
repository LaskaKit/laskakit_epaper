#pragma once

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"


namespace LaskaKit::Epaper {
    class GDEM0154F51H {
    public:
        static constexpr size_t WIDTH  = 200;
        static constexpr size_t HEIGHT = 200;
        static constexpr ColorType COLORTYPE = ColorType::BWRY;
        static constexpr const char* NAME = "GDEM0154F51H";

    private:
        static constexpr size_t frameBufferSize = WIDTH * HEIGHT / 4;
        uint8_t* frameBuffer = nullptr;

    public:
        GDEM0154F51H(const EPDBusSettings& settings)
        {}

        bool init()
        {
            this->frameBuffer = (uint8_t*)calloc(this->frameBufferSize, 1);
            return this->frameBuffer != nullptr;
        }

        ~GDEM0154F51H()
        {
            if (this->frameBuffer) {
                free(this->frameBuffer);
            }
        }

        void fullUpdate()
        {
            EPDBus::Reset();
            EPDBus::DelayMs(20);
            EPDBus::WaitBusyHigh();

            EPDBus::WriteCmdData(0x4D, {0x78});
            EPDBus::WriteCmdData(0x00, {0x0f, 0x29});
            EPDBus::WriteCmdData(0x06, {0x0d, 0x12, 0x30, 0x20, 0x19, 0x2a, 0x22});
            EPDBus::WriteCmdData(0x50, {0x37});
            EPDBus::WriteCmdData(0x61, {0x00, 0xc8, 0x00, 0xc8});  // resolution 200x200
            EPDBus::WriteCmdData(0xE9, {0x01});
            EPDBus::WriteCmdData(0x30, {0x08});
            EPDBus::WriteCmd(0x04);  // power on
            EPDBus::WaitBusyHigh();

            EPDBus::_WriteCmdData(0x10, this->frameBuffer, this->frameBufferSize);

            EPDBus::WriteCmdData(0x12, {0x00});  // display refresh
            EPDBus::WaitBusyHigh();

            EPDBus::WriteCmdData(0x50, {0x17});  // border floating
            EPDBus::WriteCmdData(0x02, {0x00});  // power off
            EPDBus::WaitBusyHigh();
            EPDBus::WriteCmdData(0x07, {0xa5});  // deep sleep
        }

        void drawPixel(int16_t x, int16_t y, uint8_t color)
        {
            size_t pos   = y * WIDTH + x;
            size_t index = pos / 4;
            size_t shift = (3 - (pos % 4)) * 2;
            uint8_t mask1 = 0b10 << shift;
            uint8_t mask2 = 0b01 << shift;

            switch (color) {
                case 0:  // black: 00
                    this->frameBuffer[index] &= ~mask1;
                    this->frameBuffer[index] &= ~mask2;
                    break;
                case 1:  // white: 01
                    this->frameBuffer[index] &= ~mask1;
                    this->frameBuffer[index] |=  mask2;
                    break;
                case 2:  // red: 11
                    this->frameBuffer[index] |=  mask1;
                    this->frameBuffer[index] |=  mask2;
                    break;
                case 3:  // yellow: 10
                    this->frameBuffer[index] |=  mask1;
                    this->frameBuffer[index] &= ~mask2;
                    break;
            }
        }
    };
}
