#pragma once

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"


namespace LaskaKit::Epaper {

class GDEY0583F41 {
public:
    static constexpr size_t WIDTH = 648;
    static constexpr size_t HEIGHT = 480;
    static constexpr ColorType COLORTYPE = ColorType::BWRY;
    static constexpr const char* NAME = "GDEY0583F41";

private:
    static constexpr size_t frameBufferSize = WIDTH * HEIGHT / 4;
    uint8_t* frameBuffer = nullptr;
public:
    GDEY0583F41(const EPDBusSettings& settings)
    {}

    bool init()
    {
        this->frameBuffer = (uint8_t*)calloc(this->frameBufferSize, 1);
        if (!this->frameBuffer) {
            return false;
        }
        return true;
    }

    ~GDEY0583F41()
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
        EPDBus::WriteCmdData(0x00, {0x2F, 0x29});
        EPDBus::WriteCmdData(0xE3, {0x88});
        EPDBus::WriteCmdData(0x50, {0x37});
        EPDBus::WriteCmdData(0x61, {WIDTH / 256, WIDTH % 256, HEIGHT / 256, HEIGHT % 256});  // resolution
        EPDBus::WriteCmdData(0x65, {0x00, 0x10, 0x00, 0x00});
        EPDBus::WriteCmdData(0xE9, {0x01});
        EPDBus::WriteCmdData(0x30, {0x08});
        EPDBus::WriteCmd(0x04);  // power on
        EPDBus::WaitBusyHigh();

        EPDBus::_WriteCmdData(0x10, this->frameBuffer, this->frameBufferSize);  // send framebuffer to screen
        EPDBus::WriteCmdData(0x12, {0x00});  // display update control
        EPDBus::WaitBusyHigh();

        EPDBus::WriteCmdData(0x02, {0x00});  // poweroff
        EPDBus::WaitBusyHigh();

        EPDBus::WriteCmdData(0x07, {0xA5});  // epd deep sleep
    }

    void drawPixel(int16_t x, int16_t y, uint8_t color)
    {
        size_t pos = y * WIDTH + x;
        size_t index = pos / 4;
        size_t shift = (3 - (pos % 4)) * 2;
        uint8_t mask1 = 0b10 << shift;
        uint8_t mask2 = 0b1 << shift;

        switch (color) {
            case 0:
                this->frameBuffer[index] &= ~mask1;
                this->frameBuffer[index] &= ~mask2;
                break;
            case 1:
                this->frameBuffer[index] &= ~mask1;
                this->frameBuffer[index] |= mask2;
                break;
            case 2:
                this->frameBuffer[index] |= mask1;
                this->frameBuffer[index] |= mask2;
                break;
            case 3:
                this->frameBuffer[index] |= mask1;
                this->frameBuffer[index] &= ~mask2;
                break;
        }
    }
};

}; // namespace LaskaKit::Epaper
