#pragma once

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

namespace LaskaKit::Epaper {
    class GDEM075F52 {
    public:
        static constexpr size_t WIDTH = 800;
        static constexpr size_t HEIGHT = 480;
        static constexpr ColorType COLORTYPE = ColorType::C4;
        static constexpr const char* NAME = "GDEM075F52";
    private:
        uint8_t* frameBuffer;
        static constexpr const size_t frameBufferSize = WIDTH * HEIGHT / 4;
    public:
        GDEM075F52(const EPDBusSettings& epdBusSettings)
        {
            EPDBus::Begin(epdBusSettings);

            this->frameBuffer = (uint8_t*)calloc(this->frameBufferSize, 1);
            if (!this->frameBuffer) {
                Serial.println("error allocating ram");
            }
        }

        ~GDEM075F52()
        {
            EPDBus::End();
            if (this->frameBuffer) {
                free(this->frameBuffer);
            }
        }

        void fullUpdate()
        {
            // EPD init
            EPDBus::Reset();

            delay(1000);  // todo lcd_chckstatus
            EPDBus::BusyWaitInv();
            EPDBus::WriteCmdData(0x00, {0x0F, 0x29});
            EPDBus::WriteCmdData(0x06, {0x0F, 0x8B, 0x93, 0xA1});
            EPDBus::WriteCmdData(0x41, {0x00});
            EPDBus::WriteCmdData(0x50, {0x37});
            EPDBus::WriteCmdData(0x60, {0x02, 0x02});
            EPDBus::WriteCmdData(0x61, {0x03, 0x20, 0x1, 0xE0});  // resolution
            EPDBus::WriteCmdData(0x62, {0x98, 0x98, 0x98, 0x75, 0xCA, 0xB2, 0x98, 0x7E});
            EPDBus::WriteCmdData(0x65, {0x00, 0x00, 0x00, 0x00});
            EPDBus::WriteCmdData(0xE7, {0x1C});
            EPDBus::WriteCmdData(0xE3, {0x00});
            EPDBus::WriteCmdData(0xE9, {0x01});
            EPDBus::WriteCmdData(0x30, {0x08});  // frame to go with waveform
            EPDBus::WriteCmd(0x04);  // power on
            EPDBus::BusyWaitInv();
            // delay(1000);  // todo lcd_chckstatus
        
            // display something
            this->writeFrameBufferToScreen();
            // EPDBus::WriteCmd(0x10);
            // uint8_t tmp;
            // for (int i = 0; i < 800 * 480 / 4; i++) {
            //     tmp = 0x55;
            //     EPDBus::_WriteData(&tmp, 1);
            // }

            // epd refresh
            EPDBus::WriteCmdData(0x12, {0x00});  // display update control
            EPDBus::BusyWaitInv();
            delay(1000);  // todo lcd_chckstatus
            
            // epd sleep
            EPDBus::WriteCmdData(0x02, {0x00});  // power off
            EPDBus::BusyWaitInv();
            delay(1000);  // todo lcd_chckstatus
            
            EPDBus::WriteCmdData(0x07, {0xA5});  // deep sleep
        }

        /* Color display description
             white  black  red  yellow
        0x10|  01     00    11    10   | frameBuffer
        */
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

    private:
        void writeFrameBufferToScreen()
        {
            EPDBus::_WriteCmdData(0x10, this->frameBuffer, this->frameBufferSize);
        }
    };

};  // namespace LaskaKit::Epaper