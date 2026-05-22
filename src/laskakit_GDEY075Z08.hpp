#pragma once

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

namespace LaskaKit::Epaper {
    class GDEY075Z08
    {
    public:
        static constexpr uint WIDTH = 800;
        static constexpr uint HEIGHT = 480;
        static constexpr ColorType COLORTYPE = ColorType::BWR;
        static constexpr const char* NAME = "GDEY075Z08";
    private:
        uint8_t* bufferBW;  // Black/White plane (0x10)
        uint8_t* bufferRW;  // Red plane (0x13)

    public:
        GDEY075Z08(const EPDBusSettings& settings)
        {}

        bool init()
        {
            this->bufferBW = (uint8_t*)malloc(48000);
            if (!this->bufferBW) {
                return false;
            }
            memset(this->bufferBW, 0xFF, 48000);  // init to white
            this->bufferRW = (uint8_t*)calloc(48000, 1);
            if (!this->bufferRW) {
                free(this->bufferBW);
                this->bufferBW = nullptr;
                return false;
            }
            return true;
        }

        ~GDEY075Z08() {
            if (this->bufferBW) {
                free(this->bufferBW);
            }
            if (this->bufferRW) {
                free(this->bufferRW);
            }
        }

        void writeBufferToScreen() {
            EPDBus::_WriteCmdData(0x10, this->bufferBW, 48000);
            EPDBus::_WriteCmdData(0x13, this->bufferRW, 48000);
        }

        void fullUpdate()
        {
            EPDBus::BeginTransaction();
            EPDBus::Reset();
            EPDBus::BusyPoll(0x71);
            EPDBus::DelayMs(200);

            EPDBus::WriteCmdData(0x01, {0x07, 0x07, 0x3f, 0x3f});  // power setting, VGH=20V VGL=-20V VDH=15V VDL=-15V
            EPDBus::WriteCmdData(0x06, {0x17, 0x17, 0x28, 0x17});  // booster soft start
            EPDBus::WriteCmd(0x04);  // power on
            EPDBus::BusyPoll(0x71);
            EPDBus::DelayMs(200);

            EPDBus::WriteCmdData(0x00, {0x0F});              // panel setting, KWR mode
            EPDBus::WriteCmdData(0x61, {0x03, 0x20, 0x01, 0xE0});  // resolution 800x480
            EPDBus::WriteCmdData(0x15, {0x00});
            EPDBus::WriteCmdData(0x50, {0x11, 0x07});        // VCOM and data interval
            EPDBus::WriteCmdData(0x60, {0x22});              // TCON setting

            this->writeBufferToScreen();

            EPDBus::WriteCmd(0x12);   // display refresh
            EPDBus::DelayMs(1);
            EPDBus::BusyPoll(0x71);
            EPDBus::DelayMs(200);

            EPDBus::WriteCmdData(0x50, {0xf7});  // VCOM and data interval setting
            EPDBus::WriteCmd(0x02);   // power off
            EPDBus::BusyPoll(0x71);
            EPDBus::DelayMs(100);
            EPDBus::WriteCmdData(0x07, {0xA5});  // deep sleep
            EPDBus::EndTransaction();
        }

        /* Color display description
             white  black  red
        0x10|  1      0      0
        0x13|  0      0      1
        */
        void drawPixel(int16_t x, int16_t y, uint8_t color)
        {
            if (color > 0x2) {
                return;
            }
            size_t pos = y * WIDTH + x;
            size_t index = pos / 8;
            size_t shift = 7 - (pos % 8);
            uint8_t mask = 0b1 << shift;

            switch (color) {
                case 0:  // black
                    this->bufferBW[index] &= ~mask;
                    this->bufferRW[index] &= ~mask;
                    break;
                case 1:  // white
                    this->bufferBW[index] |= mask;
                    this->bufferRW[index] &= ~mask;
                    break;
                case 2:  // red
                    this->bufferBW[index] &= ~mask;
                    this->bufferRW[index] |= mask;
                    break;
            }
        }
    };
}
