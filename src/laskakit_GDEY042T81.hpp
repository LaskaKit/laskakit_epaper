#pragma once

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"

namespace LaskaKit::Epaper {

class GDEY042T81 {
public:
    static constexpr size_t WIDTH = 400;
    static constexpr size_t HEIGHT = 300;
    static constexpr ColorType COLORTYPE = ColorType::G4;
    static constexpr const char* NAME = "GDEY042T81";

private:
    static constexpr size_t frameBufferSize = WIDTH * HEIGHT / 8;
    uint8_t* bufferBW;   // RAM 0x24
    uint8_t* bufferGray; // RAM 0x26

    // 4-gray LUT - from GxEPD2_4G reference (GxEPD2_420_GDEY042T81)
    static constexpr uint8_t lut_4G[] = {
        0x01, 0x0A, 0x1B, 0x0F, 0x03, 0x01, 0x01, // VS VCOM
        0x05, 0x0A, 0x01, 0x0A, 0x01, 0x01, 0x01,
        0x05, 0x08, 0x03, 0x02, 0x04, 0x01, 0x01,
        0x01, 0x04, 0x04, 0x02, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x0A, 0x1B, 0x0F, 0x03, 0x01, 0x01, // VS WW
        0x05, 0x4A, 0x01, 0x8A, 0x01, 0x01, 0x01,
        0x05, 0x48, 0x03, 0x82, 0x84, 0x01, 0x01,
        0x01, 0x84, 0x84, 0x82, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x10, 0x28, 0x97, 0x03, 0x01, 0x01, // VS BW
        0x05, 0x4A, 0x01, 0x8A, 0x01, 0x01, 0x01,
        0x05, 0x48, 0x83, 0x82, 0x04, 0x01, 0x01,
        0x01, 0x04, 0x04, 0x02, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x90, 0x28, 0x97, 0x03, 0x01, 0x01, // VS WB
        0x05, 0x4A, 0x01, 0x8A, 0x01, 0x01, 0x01,
        0x05, 0x48, 0x83, 0x02, 0x04, 0x01, 0x01,
        0x01, 0x04, 0x04, 0x02, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x8A, 0x9B, 0x8F, 0x03, 0x01, 0x01, // VS BB
        0x05, 0x4A, 0x01, 0x8A, 0x01, 0x01, 0x01,
        0x05, 0x48, 0x03, 0x42, 0x04, 0x01, 0x01,
        0x01, 0x04, 0x04, 0x42, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00,                         // end of LUT @224
        0x07, 0x17, 0x41, 0xA8, 0x32, 0x30,   // 0x3F, VGH, VSH1, VSH2, VSL, VCOM
    };

public:
    GDEY042T81(const EPDBusSettings& settings)
    {}

    bool init()
    {
        this->bufferBW = (uint8_t*)calloc(this->frameBufferSize, 1);
        if (!this->bufferBW) {
            return false;
        }
        this->bufferGray = (uint8_t*)calloc(this->frameBufferSize, 1);
        if (!this->bufferGray) {
            free(this->bufferBW);
            this->bufferBW = nullptr;
            return false;
        }
        return true;
    }

    ~GDEY042T81()
    {
        EPDBus::End();
        if (this->bufferBW) {
            free(this->bufferBW);
        }
        if (this->bufferGray) {
            free(this->bufferGray);
        }
    }

    void fullUpdate()
    {
        EPDBus::BeginTransaction();
        EPDBus::Reset();
        EPDBus::DelayMs(10);

        EPDBus::WriteCmd(0x12); // sw reset
        EPDBus::DelayMs(10);
        EPDBus::BusyWait();

        EPDBus::WriteCmdData(0x0C, {0x8B, 0x9C, 0xA4, 0x0F}); // soft start
        EPDBus::WriteCmdData(0x21, {0x00, 0x00});
        EPDBus::WriteCmdData(0x3C, {0x03}); // border

        // set RAM area (full screen)
        EPDBus::WriteCmdData(0x11, {0x03}); // entry mode: x+, y+
        EPDBus::WriteCmdData(0x44, {0x00, (uint8_t)((WIDTH - 1) / 8)});
        EPDBus::WriteCmdData(0x45, {0x00, 0x00, (uint8_t)((HEIGHT - 1) % 256), (uint8_t)((HEIGHT - 1) / 256)});
        EPDBus::WriteCmdData(0x4E, {0x00});
        EPDBus::WriteCmdData(0x4F, {0x00, 0x00});

        // load 4-gray LUT
        EPDBus::_WriteCmdData(0x32, lut_4G, 227);
        EPDBus::WriteCmdData(0x3F, {lut_4G[227]});
        EPDBus::WriteCmdData(0x03, {lut_4G[228]}); // VGH
        EPDBus::WriteCmdData(0x04, {lut_4G[229], lut_4G[230], lut_4G[231]}); // VSH1, VSH2, VSL
        EPDBus::WriteCmdData(0x2C, {lut_4G[232]}); // VCOM

        // write both RAM planes
        EPDBus::_WriteCmdData(0x24, this->bufferBW, this->frameBufferSize);
        EPDBus::_WriteCmdData(0x26, this->bufferGray, this->frameBufferSize);

        // 4-gray update
        EPDBus::WriteCmdData(0x21, {0x88, 0x00}); // b/w inverted, RED inverted
        EPDBus::WriteCmdData(0x22, {0xCF});
        EPDBus::WriteCmd(0x20);
        EPDBus::BusyWait();

        EPDBus::WriteCmdData(0x10, {0x01}); // deep sleep
        EPDBus::EndTransaction();
    }

    /* Gray level mapping:
         white  lightgray  darkgray  black
    0x24|  0        1         0        1  | BW
    0x26|  0        0         1        1  | Gray
    */
    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        size_t pos = y * WIDTH + x;
        size_t index = pos / 8;
        size_t shift = pos % 8;
        uint8_t mask = 0b1 << (7 - shift);

        if (color == RGB565::WHITE) {
            this->bufferBW[index] &= ~mask;
            this->bufferGray[index] &= ~mask;
        }

        if (color == RGB565::BLACK) {
            this->bufferBW[index] |= mask;
            this->bufferGray[index] |= mask;
        }

        if (color == RGB565::LIGHT_GRAY) {
            this->bufferBW[index] |= mask;
            this->bufferGray[index] &= ~mask;
        }

        if (color == RGB565::DARK_GRAY) {
            this->bufferBW[index] &= ~mask;
            this->bufferGray[index] |= mask;
        }
    }
};

}; // namespace LaskaKit::Epaper
