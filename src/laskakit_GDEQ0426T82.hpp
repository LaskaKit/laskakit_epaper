#pragma once

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"

namespace LaskaKit::Epaper {

class GDEQ0426T82 {
public:
    static constexpr size_t WIDTH = 480;
    static constexpr size_t HEIGHT = 800;
    static constexpr ColorType COLORTYPE = ColorType::BW;
    static constexpr const char* NAME = "GDEQ0426T82";

private:
    static constexpr size_t frameBufferSize = WIDTH * HEIGHT / 8;
    uint8_t* frameBuffer;

public:
    GDEQ0426T82(const EPDBusSettings& settings)
    {
        EPDBus::Begin(settings);
        this->frameBuffer = (uint8_t*)calloc(this->frameBufferSize, 1);
        if (!this->frameBuffer) {
            Serial.println("error allocating ram");
        }
    }

    ~GDEQ0426T82()
    {
        EPDBus::End();
        if (this->frameBuffer) {
            free(this->frameBuffer);
        }
    }

    void fullUpdate()
    {
        EPDBus::BeginTransaction();
        EPDBus::Reset();
        EPDBus::DelayMs(20);
        EPDBus::BusyWait();

        EPDBus::WriteCmd(0x12); // sw reset
        EPDBus::BusyWait();

        EPDBus::WriteCmdData(0x18, {0x80});
        EPDBus::WriteCmdData(0x0C, {0xAE, 0xC7, 0xC3, 0xC0, 0x80});
        EPDBus::WriteCmdData(0x01, {(WIDTH - 1) % 256, (WIDTH - 1) / 256, 0x02});
        EPDBus::WriteCmdData(0x3C, {0x01});
        EPDBus::WriteCmdData(0x11, {0x03});
        EPDBus::WriteCmdData(0x44, {0x00, 0x00, (HEIGHT - 1) % 256, (HEIGHT - 1) / 256});
        EPDBus::WriteCmdData(0x4E, {0x00, 0x00});  // ram x address count
        EPDBus::WriteCmdData(0x4F, {0x00, 0x00});  // ram y address count
        EPDBus::BusyWait();

        EPDBus::_WriteCmdData(0x24, this->frameBuffer, this->frameBufferSize);
        EPDBus::WriteCmdData(0x22, {0xF7});
        EPDBus::WriteCmd(0x20);
        EPDBus::BusyWait();

        EPDBus::WriteCmdData(0x10, {0x01});
        EPDBus::EndTransaction();
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        size_t pos = x * HEIGHT + y;
        size_t index = pos / 8;
        size_t shift = pos % 8;
        uint8_t mask = 0b1 << (7 - shift);

        if (color == RGB565::WHITE) {
            this->frameBuffer[index] |= mask;
        }

        if (color == RGB565::BLACK) {
            this->frameBuffer[index] &= ~mask;
        }
    }
};

}; // namespace LaskaKit::Epaper
