#pragma once

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"

namespace LaskaKit::Epaper {

class GDEQ0583T81 {
public:
    static constexpr size_t WIDTH = 648;
    static constexpr size_t HEIGHT = 480;
    static constexpr ColorType COLORTYPE = ColorType::BW;
    static constexpr const char* NAME = "GDEQ0583T81";

private:
    static constexpr size_t frameBufferSize = WIDTH * HEIGHT / 8;
    uint8_t* bufferBW;

public:
    GDEQ0583T81(const EPDBusSettings& settings)
    {}

    bool init()
    {
        this->bufferBW = (uint8_t*)calloc(this->frameBufferSize, 1);
        if (!this->bufferBW) {
            return false;
        }
        return true;
    }

    ~GDEQ0583T81()
    {
        if (this->bufferBW) {
            free(this->bufferBW);
        }
    }

    void fullUpdate()
    {
        EPDBus::BeginTransaction();

        // hardware reset
        EPDBus::Reset();

        // panel setting
        EPDBus::WriteCmdData(0x00, {0x1F});

        // power on
        EPDBus::WriteCmd(0x04);
        EPDBus::DelayMs(300);
        EPDBus::BusyWaitInv();

        // VCOM and data interval setting
        EPDBus::WriteCmdData(0x50, {0x21, 0x07});

        // write old data (all white)
        EPDBus::WriteCmd(0x10);
        uint8_t chunk[256];
        memset(chunk, 0xFF, sizeof(chunk));
        for (size_t i = 0; i < frameBufferSize; i += sizeof(chunk)) {
            size_t len = sizeof(chunk);
            if (i + len > frameBufferSize) {
                len = frameBufferSize - i;
            }
            EPDBus::_WriteData(chunk, len);
        }

        // write new data (frame buffer)
        EPDBus::_WriteCmdData(0x13, this->bufferBW, this->frameBufferSize);

        // display refresh
        EPDBus::WriteCmd(0x12);
        EPDBus::DelayMs(1);
        EPDBus::BusyWaitInv();

        // deep sleep
        EPDBus::WriteCmdData(0x50, {0xF7});
        EPDBus::WriteCmd(0x02); // power off
        EPDBus::BusyWaitInv();
        EPDBus::DelayMs(100);
        EPDBus::WriteCmdData(0x07, {0xA5}); // deep sleep

        EPDBus::EndTransaction();
    }

    // BW mapping (display native: 1=white, 0=black)
    void drawPixel(int16_t x, int16_t y, uint8_t color)
    {
        size_t pos = y * WIDTH + x;
        size_t index = pos / 8;
        size_t shift = pos % 8;
        uint8_t mask = 0b1 << (7 - shift);

        switch (color) {
            case 0:
                this->bufferBW[index] &= ~mask;
                break;
            case 1:
                this->bufferBW[index] |= mask;
                break;
        }
    }
};

}; // namespace LaskaKit::Epaper
