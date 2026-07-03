#pragma once

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"


namespace LaskaKit::Epaper {
    class GDEP073E01 {
    public:
        static constexpr size_t WIDTH  = 800;
        static constexpr size_t HEIGHT = 480;
        static constexpr ColorType COLORTYPE = ColorType::C7;
        static constexpr const char* NAME = "GDEP073E01";

    private:
        static constexpr size_t frameBufferSize = WIDTH * HEIGHT / 2;
        uint8_t* frameBuffer = nullptr;

        // Spectra 6 panel pixel values, two pixels per byte (high nibble first)
        static constexpr uint8_t PANEL_BLACK  = 0x0;
        static constexpr uint8_t PANEL_WHITE  = 0x1;
        static constexpr uint8_t PANEL_YELLOW = 0x2;
        static constexpr uint8_t PANEL_RED    = 0x3;
        static constexpr uint8_t PANEL_ORANGE = 0x4;
        static constexpr uint8_t PANEL_BLUE   = 0x5;
        static constexpr uint8_t PANEL_GREEN  = 0x6;

    public:
        GDEP073E01(const EPDBusSettings& settings)
        {}

        bool init()
        {
            this->frameBuffer = (uint8_t*)malloc(this->frameBufferSize);
            if (!this->frameBuffer) {
                return false;
            }
            memset(this->frameBuffer, (PANEL_WHITE << 4) | PANEL_WHITE, this->frameBufferSize);
            return true;
        }

        ~GDEP073E01()
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

            EPDBus::WriteCmdData(0xAA, {0x49, 0x55, 0x20, 0x08, 0x09, 0x18});  // CMDH
            EPDBus::WriteCmdData(0x01, {0x3F});  // PWR
            EPDBus::WriteCmdData(0x00, {0x5F, 0x69});  // PSR
            EPDBus::WriteCmdData(0x03, {0x00, 0x54, 0x00, 0x44});  // POFS
            EPDBus::WriteCmdData(0x05, {0x40, 0x1F, 0x1F, 0x2C});  // BTST1
            EPDBus::WriteCmdData(0x06, {0x6F, 0x1F, 0x17, 0x49});  // BTST2
            EPDBus::WriteCmdData(0x08, {0x6F, 0x1F, 0x1F, 0x22});  // BTST3
            EPDBus::WriteCmdData(0x30, {0x08});  // PLL
            EPDBus::WriteCmdData(0x50, {0x3F});  // CDI
            EPDBus::WriteCmdData(0x60, {0x02, 0x00});  // TCON
            EPDBus::WriteCmdData(0x61, {WIDTH / 256, WIDTH % 256, HEIGHT / 256, HEIGHT % 256});  // resolution
            EPDBus::WriteCmdData(0x84, {0x01});  // T_VDCS
            EPDBus::WriteCmdData(0xE3, {0x2F});  // PWS
            EPDBus::WriteCmd(0x04);  // power on
            EPDBus::WaitBusyHigh();

            EPDBus::_WriteCmdData(0x10, this->frameBuffer, this->frameBufferSize);

            EPDBus::WriteCmdData(0x12, {0x00});  // display refresh
            EPDBus::WaitBusyHigh();

            EPDBus::WriteCmdData(0x02, {0x00});  // power off
            EPDBus::WaitBusyHigh();
            EPDBus::WriteCmdData(0x07, {0xA5});  // deep sleep
        }

        void drawPixel(int16_t x, int16_t y, uint8_t color)
        {
            size_t pos   = y * WIDTH + x;
            size_t index = pos / 2;
            size_t shift = (pos % 2) ? 0 : 4;

            if (index >= this->frameBufferSize) {
                return;
            }

            uint8_t panelColor;
            switch (color) {
                case 0:
                    panelColor = PANEL_BLACK;
                    break;
                case 1:
                    panelColor = PANEL_WHITE;
                    break;
                case 2:
                    panelColor = PANEL_RED;
                    break;
                case 3:
                    panelColor = PANEL_YELLOW;
                    break;
                case 4:
                    panelColor = PANEL_GREEN;
                    break;
                case 5:
                    panelColor = PANEL_BLUE;
                    break;
                case 6:
                    panelColor = PANEL_ORANGE;
                    break;
                default:
                    return;
            }
            this->frameBuffer[index] = (this->frameBuffer[index] & ~(0xF << shift)) | (panelColor << shift);
        }
    };
}
