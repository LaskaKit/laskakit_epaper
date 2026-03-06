#pragma once

#include "epdbus.hpp"
#include "laskakit_epaper.hpp"

namespace LaskaKit::Epaper {

class GDEQ0426T82 {
public:
    static constexpr size_t WIDTH = 800;
    static constexpr size_t HEIGHT = 480;
    static constexpr ColorType COLORTYPE = ColorType::G4;
    static constexpr const char* NAME = "GDEQ0426T82";

private:
    static constexpr size_t frameBufferSize = WIDTH * HEIGHT / 8;
    uint8_t* bufferBW;   // RAM 0x24
    uint8_t* bufferGray; // RAM 0x26

    // 4-gray LUT - from GxEPD2_4G reference
    static constexpr uint8_t lut_4G[] = {
        0x80, 0x48, 0x4A, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // VS L0 (red 0, black 0) white
        0x0A, 0x48, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // VS L1 (red 1, black 0) light grey
        0x88, 0x48, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // VS L2 (red 0, black 1) dark grey
        0xA8, 0x48, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // VS L3 (red 1, black 1) black
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // VS L4 vcom
        0x07, 0x1E, 0x1C, 0x02, 0x00, // TP 0 RP0 @50
        0x05, 0x01, 0x05, 0x01, 0x02, // TP 1 RP1
        0x08, 0x01, 0x01, 0x04, 0x04, // TP 2 RP2
        //0x00, 0x02, 0x00, 0x02, 0x02, // TP 3 RP3
        0x00, 0x02, 0x01, 0x02, 0x02, // TP 3 RP3
        0x00, 0x00, 0x00, 0x00, 0x00, // TP 4 RP4
        0x00, 0x00, 0x00, 0x00, 0x00, // TP 5 RP5
        0x00, 0x00, 0x00, 0x00, 0x00, // TP 6 RP6
        0x00, 0x00, 0x00, 0x00, 0x00, // TP 7 RP7
        0x00, 0x00, 0x00, 0x00, 0x00, // TP 8 RP8
        0x00, 0x00, 0x00, 0x00, 0x01, // TP 9 RP9
        0x22, 0x22, 0x22, 0x22, 0x22, // frame rate
        0x17, 0x41, 0xA8, 0x32, 0x30, // VGH, VSH1, VSH2, VSL, VCOM
        0x00, 0x00, // Reserve
    };

public:
    GDEQ0426T82(const EPDBusSettings& settings)
    {
        EPDBus::Begin(settings);
        this->bufferBW = (uint8_t*)calloc(this->frameBufferSize, 1);
        this->bufferGray = (uint8_t*)calloc(this->frameBufferSize, 1);
        if (!this->bufferBW || !this->bufferGray) {
            Serial.println("error allocating ram");
        }
    }

    ~GDEQ0426T82()
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
        EPDBus::DelayMs(20);
        EPDBus::BusyWait();

        EPDBus::WriteCmd(0x12); // sw reset
        EPDBus::DelayMs(10);
        EPDBus::BusyWait();

        EPDBus::WriteCmdData(0x0C, {0xAE, 0xC7, 0xC3, 0xC0, 0x80});
        EPDBus::WriteCmdData(0x01, {(HEIGHT - 1) % 256, (HEIGHT - 1) / 256, 0x02});
        EPDBus::WriteCmdData(0x3C, {0x00}); // border: LUT0 (white)
        EPDBus::WriteCmdData(0x18, {0x80}); // internal temperature sensor
        EPDBus::WriteCmdData(0x11, {0x03});
        EPDBus::WriteCmdData(0x44, {0x00, 0x00, (WIDTH - 1) % 256, (WIDTH - 1) / 256});
        EPDBus::WriteCmdData(0x45, {0x00, 0x00, (HEIGHT - 1) % 256, (HEIGHT - 1) / 256});
        EPDBus::WriteCmdData(0x4E, {0x00, 0x00});
        EPDBus::WriteCmdData(0x4F, {0x00, 0x00});
        EPDBus::BusyWait();

        // load 4-gray LUT
        EPDBus::_WriteCmdData(0x32, lut_4G, 105);
        EPDBus::WriteCmdData(0x03, {lut_4G[105]}); // VGH
        EPDBus::WriteCmdData(0x04, {lut_4G[106], lut_4G[107], lut_4G[108]}); // VSH1, VSH2, VSL
        EPDBus::WriteCmdData(0x2C, {lut_4G[109]}); // VCOM

        // write both RAM planes
        EPDBus::_WriteCmdData(0x24, this->bufferBW, this->frameBufferSize);
        EPDBus::_WriteCmdData(0x26, this->bufferGray, this->frameBufferSize);

        // 4-gray update
        EPDBus::WriteCmdData(0x21, {0x00, 0x00});
        EPDBus::WriteCmdData(0x22, {0xC7});
        EPDBus::WriteCmd(0x20);
        EPDBus::BusyWait();

        EPDBus::WriteCmdData(0x10, {0x01});
        EPDBus::EndTransaction();
    }

    /* Gray level mapping:
         white  lightgray  darkgray  black
    0x24|  0        1         0        1  | BW
    0x26|  0        0         1        1  | Gray
    */
    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        size_t pos = (HEIGHT - 1 - y) * WIDTH + x;
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
