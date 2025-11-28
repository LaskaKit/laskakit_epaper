#pragma once

#include "laskakit_epaper.hpp"

#include "epdbus.hpp"

namespace LaskaKit::Epaper {
    class GDEY075T7
    {
    public:
        static constexpr uint WIDTH = 800;
        static constexpr uint HEIGHT = 480;
    private:
        uint8_t* bufferOld;
        uint8_t* bufferNew;

        static const uint8_t lut_vcom[42];
        static const uint8_t lut_ww[42];
        static const uint8_t lut_bw[42];
        static const uint8_t lut_wb[42];
        static const uint8_t lut_bb[42];

    public:
        GDEY075T7(const EPDBusSettings& epdBusSettings)
        {
            EPDBus::Begin(
                epdBusSettings.sck,
                epdBusSettings.mosi,
                epdBusSettings.cs,
                epdBusSettings.dc,
                epdBusSettings.busy,
                epdBusSettings.reset
            );
            this->bufferOld = (uint8_t*)malloc(48000);
            this->bufferNew = (uint8_t*)malloc(48000);
            if (!this->bufferOld || !this->bufferNew) {
                Serial.println("error allocating ram");
            }

            this->setupBuffer();
        }

        ~GDEY075T7() {
            if (this->bufferOld) {
                free(this->bufferOld);
            }
            if (this->bufferNew) {
                free(this->bufferNew);
            }
        }

        void setupBuffer()
        {
            // for (int i = 0; i < 48000; i++) {
            //     this->bufferOld[i] = 0xFF;
            //     this->bufferNew[i] = 0x00;
            // }

            for (int row = 0; row < 480; row++) {
                for (int col = 0; col < 800; col += 8) {
                    int index = (row * 800 + col) / 8;
                    uint8_t tmpOld = 0;
                    uint8_t tmpNew = 0;

                    for (int bit = 0; bit < 8; bit++) {
                        tmpOld <<= 1;
                        tmpNew <<= 1;
                        if (row < 120) {
                            // Top quarter: White
                            tmpOld |= 0x1;
                            tmpNew |= 0x1;
                        } else if (row < 240) {
                            // Second quarter: Light Gray
                            tmpOld |= 0x0;
                            tmpNew |= 0x1;
                        } else if (row < 360) {
                            // Third quarter: Dark Gray
                            tmpOld |= 0x1;
                            tmpNew |= 0x0;
                        } else {
                            // Bottom quarter: Black
                            tmpOld |= 0x0;
                            tmpNew |= 0x0;
                        }
                    }
                    this->bufferOld[index] = tmpOld;
                    this->bufferNew[index] = tmpNew;
                    // printf("index: %lu\n", index);
                }
            }
        }


        //4 grayscale demo function
        /********Color display description
             white  gray1  gray2  black
        0x10|  01     01     00     00
        0x13|  01     00     01     00
        ****************/
        void writeBufferToScreen() {
            EPDBus::_WriteCmdData(0x10, this->bufferOld, 48000);
            EPDBus::_WriteCmdData(0x13, this->bufferNew, 48000);
        }

        void fullUpdate()
        {
            EPDBus::BeginTransaction();
            EPDBus::Reset();

            EPDBus::WriteCmdData(0x06, {0x17, 0x17, 0x28, 0x17});  // booster soft start
            EPDBus::WriteCmd(0x04);  // power on
            EPDBus::DelayMs(100);
            EPDBus::BusyPoll(0x71);  // waiting for the electronic paper IC to release the idle signal
            EPDBus::WriteCmdData(0x00, {0xBF});
            EPDBus::WriteCmdData(0x30, {0x06});
            EPDBus::WriteCmdData(0x61, {0x03, 0x20, 0x01, 0xE0});  // set resolution
            EPDBus::WriteCmdData(0x15, {0x00});
            EPDBus::WriteCmdData(0x60, {0x22});  // TCON setting
            EPDBus::WriteCmdData(0x82, {0x12});  // vcom_DC setting
            EPDBus::WriteCmdData(0x50, {0x10, 0x07});  // vcom and data interval setting, 10:KW(0--1)  21:KW(1--0)

            // write buffers to screen
            this->writeBufferToScreen();

            // write some magic lookup tables
            this->lut();

            // display refresh
            EPDBus::WriteCmd(0x12);  // display refresh
            EPDBus::DelayMs(100);  // delay here is necessary, 200uS at least!!!
            EPDBus::BusyPoll(0x71);

            // epd sleep
            EPDBus::WriteCmdData(0x50, {0xF7});  // vcom and data interval setting
            EPDBus::WriteCmd(0x02);  // power off
            EPDBus::BusyPoll(0x71);
            EPDBus::WriteCmdData(0x07, {0xA5});
            EPDBus::EndTransaction();
        }

        void drawPixel(int16_t x, int16_t y, uint32_t color)
        {
            size_t pos = y * 800 + x;
            size_t index = pos / 8;
            size_t shift = 7 - (pos % 8);
            // printf("%d %d %lu %lu\n", x, y, index, shift);
            if (color & 0b10) {
                this->bufferNew[index] &= ~(0xFF & (0b1 << shift));
            } else {
                this->bufferNew[index] |= (0b1 << shift);
            }

            if (color & 0b01) {
                this->bufferOld[index] &= ~(0xFF & (0b1 << shift));
            } else {
                this->bufferOld[index] |= (0b1 << shift);
            }
            // this->buffer[pos] = color;
            // printf("%lu\n", shift);
        }

    private:
        void lut()
        {
            EPDBus::_WriteCmdData(0x20, lut_vcom, 42);  // vcom
            EPDBus::_WriteCmdData(0x21, lut_ww, 42);  // red not use
            EPDBus::_WriteCmdData(0x22, lut_bw, 42);  // bw r
            EPDBus::_WriteCmdData(0x23, lut_wb, 42);  // wb r
            EPDBus::_WriteCmdData(0x24, lut_bb, 42);  // bb b
            EPDBus::_WriteCmdData(0x25, lut_ww, 42);  // vcom
        }
    };


    // 0~3 gray
    const uint8_t GDEY075T7::lut_vcom[42] = {
        0x00, 0x0A, 0x00, 0x00, 0x00, 0x01,
        0x60, 0x14, 0x14, 0x00, 0x00, 0x01,
        0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x13, 0x0A, 0x01, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

     // R21
    const uint8_t GDEY075T7::lut_ww[42] = {
        0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
        0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
        0x10, 0x14, 0x0A, 0x00, 0x00, 0x01,
        0xA0, 0x13, 0x01, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // R22H  r
    const uint8_t GDEY075T7::lut_bw[42] = {
        0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
        0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
        0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
        0x99, 0x0C, 0x01, 0x03, 0x04, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // R23H  w
    const uint8_t GDEY075T7::lut_wb[42] = {
        0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
        0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
        0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
        0x99, 0x0B, 0x04, 0x04, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // R24H  b
    const uint8_t GDEY075T7::lut_bb[42] = {
        0x80, 0x0A, 0x00, 0x00, 0x00, 0x01,
        0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
        0x20, 0x14, 0x0A, 0x00, 0x00, 0x01,
        0x50, 0x13, 0x01, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
}
