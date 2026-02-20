#pragma once

#include <GxEPD2.h>
#include <GxEPD2_BW.h>

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {

class GDEY0583F41 {
public:
    static constexpr size_t WIDTH = 648;
    static constexpr size_t HEIGHT = 480;
    static constexpr ColorType COLORTYPE = ColorType::C4;
    static constexpr const char* NAME = "GDEY0583F41";

private:
    static constexpr size_t frameBufferSize = WIDTH * HEIGHT / 4;
    uint8_t* frameBuffer;
public:
    GDEY0583F41(const EPDBusSettings& settings)
    {
        EPDBus::Begin(settings);
        this->frameBuffer = (uint8_t*)calloc(this->frameBufferSize, 1);
        if (!this->frameBuffer) {
            Serial.println("error allocating ram");
        }
    }

    ~GDEY0583F41()
    {
        EPDBus::End();
        if (this->frameBuffer) {
            free(this->frameBuffer);
        }
    }

    void fullUpdate()
    {
        EPDBus::Reset();
        EPDBus::DelayMs(20);
        EPDBus::BusyWaitInv();

        EPDBus::WriteCmdData(0x4D, {0x78});
        EPDBus::WriteCmdData(0x00, {0x2F, 0x29});
        EPDBus::WriteCmdData(0xE3, {0x88});
        EPDBus::WriteCmdData(0x50, {0x37});
        EPDBus::WriteCmdData(0x61, {WIDTH / 256, WIDTH % 256, HEIGHT / 256, HEIGHT % 256});  // resolution
        EPDBus::WriteCmdData(0x65, {0x00, 0x10, 0x00, 0x00});
        EPDBus::WriteCmdData(0xE9, {0x01});
        EPDBus::WriteCmdData(0x30, {0x08});
        EPDBus::WriteCmd(0x04);  // power on
        EPDBus::BusyWaitInv();

        EPDBus::_WriteCmdData(0x10, this->frameBuffer, this->frameBufferSize);  // send framebuffer to screen
        EPDBus::WriteCmdData(0x12, {0x00});  // display update control
        EPDBus::BusyWaitInv();

        EPDBus::WriteCmdData(0x02, {0x00});  // poweroff
        EPDBus::BusyWaitInv();

        EPDBus::WriteCmdData(0x07, {0xA5});  // epd deep sleep
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        size_t pos = y * WIDTH + x;
        size_t index = pos / 4;
        size_t shift = (3 - (pos % 4)) * 2;
        uint8_t mask1 = 0b10 << shift;
        uint8_t mask2 = 0b1 << shift;

        if (color == RGB565::WHITE) {
            this->frameBuffer[index] &= ~mask1;
            this->frameBuffer[index] |= mask2;
        }

        if (color == RGB565::BLACK) {
            this->frameBuffer[index] &= ~mask1;
            this->frameBuffer[index] &= ~mask2;
        }

        if (color == RGB565::RED) {
            this->frameBuffer[index] |= mask1;
            this->frameBuffer[index] |= mask2;
        }

        if (color == RGB565::YELLOW) {
            this->frameBuffer[index] |= mask1;
            this->frameBuffer[index] &= ~mask2;
        }
    }
};

}; // namespace LaskaKit::Epaper
