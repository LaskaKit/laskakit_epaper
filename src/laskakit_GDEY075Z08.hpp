#pragma once

#include <laskakit_epaper.hpp>
#include <raw2.hpp>

namespace LaskaKit::Epaper {
    class GDEY075Z08 : public Display
    {
    public:
        static constexpr uint WIDTH = 800;
        static constexpr uint HEIGHT = 480;
    private:
        uint8_t* bufferBW;  // Black/White plane (0x10)
        uint8_t* bufferRW;  // Red/White plane (0x13)

    public:
        GDEY075Z08()
        {
            this->bufferBW = (uint8_t*)malloc(48000);
            this->bufferRW = (uint8_t*)malloc(48000);
            if (!this->bufferBW || !this->bufferRW) {
                Serial.println("error allocating ram");
            }

            delay(500);
            pinMode(PIN_EPD_BUSY, INPUT);
            pinMode(PIN_EPD_RST, OUTPUT);
            pinMode(PIN_EPD_DC, OUTPUT);
            pinMode(PIN_EPD_CS, OUTPUT);
            //SPI
            SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
            SPI.begin (PIN_EPD_SCL, -1, PIN_EPD_SDA, PIN_EPD_CS);
            this->setupBuffer();
        }

        ~GDEY075Z08() {
            SPI.end();
            if (this->bufferBW) {
                free(this->bufferBW);
            }
            if (this->bufferRW) {
                free(this->bufferRW);
            }
        }

        void setupBuffer()
        {
            // Create test pattern with Red/Black/White stripes
            for (int row = 0; row < 480; row++) {
                for (int col = 0; col < 800; col += 8) {
                    int index = (row * 800 + col) / 8;
                    uint8_t tmpBW = 0;
                    uint8_t tmpRW = 0;

                    for (int bit = 0; bit < 8; bit++) {
                        tmpBW <<= 1;
                        tmpRW <<= 1;
                        if (row < 160) {
                            // Top third: White (BW=1, RW=0)
                            tmpBW |= 0x1;
                            tmpRW |= 0x0;
                        } else if (row < 320) {
                            // Middle third: Black (BW=0, RW=0)
                            tmpBW |= 0x0;
                            tmpRW |= 0x0;
                        } else {
                            // Bottom third: Red (BW=0, RW=1)
                            tmpBW |= 0x0;
                            tmpRW |= 0x1;
                        }
                    }
                    this->bufferBW[index] = tmpBW;
                    this->bufferRW[index] = tmpRW;
                }
            }
        }


        //3 color display (Red/Black/White)
        /********Color display description
             white  black  red
        0x10|  1      0      0   (inverted when sent)
        0x13|  0      0      1

        Color encoding: 0b[BW][RW]
        - White = 0b10
        - Black = 0b00
        - Red   = 0b01
                                        ****************/
        void writeBufferToScreen() {
            // BW plane (0x10) - must be inverted
            EPD_W21_WriteCMD(0x10);
            for (size_t i = 0; i < 48000; i++) {
                EPD_W21_WriteDATA(~this->bufferBW[i]);  // Invert BW data
            }

            // RW plane (0x13)
            EPD_W21_WriteCMD(0x13);
            for (size_t i = 0; i < 48000; i++) {
                EPD_W21_WriteDATA(this->bufferRW[i]);
            }
        }

        void EPD_Init_RBW()
        {
            EPD_W21_RST_0;  // Module reset
            delay(10);      // At least 10ms delay
            EPD_W21_RST_1;
            delay(10);      // At least 10ms delay
            lcd_chkstatus();

            EPD_W21_WriteCMD(0x01);     // POWER SETTING
            EPD_W21_WriteDATA(0x07);
            EPD_W21_WriteDATA(0x07);    // VGH=20V,VGL=-20V
            EPD_W21_WriteDATA(0x3f);    // VDH=15V
            EPD_W21_WriteDATA(0x3f);    // VDL=-15V

            // Enhanced display drive
            EPD_W21_WriteCMD(0x06);     // Booster Soft Start
            EPD_W21_WriteDATA(0x17);
            EPD_W21_WriteDATA(0x17);
            EPD_W21_WriteDATA(0x28);
            EPD_W21_WriteDATA(0x17);

            EPD_W21_WriteCMD(0x04);     // POWER ON
            lcd_chkstatus();            // waiting for the electronic paper IC to release the idle signal

            EPD_W21_WriteCMD(0x00);     // PANEL SETTING
            EPD_W21_WriteDATA(0x0F);    // KWR mode for Red/Black/White

            EPD_W21_WriteCMD(0x61);     // resolution setting
            EPD_W21_WriteDATA(WIDTH / 256);
            EPD_W21_WriteDATA(WIDTH % 256);
            EPD_W21_WriteDATA(HEIGHT / 256);
            EPD_W21_WriteDATA(HEIGHT % 256);

            EPD_W21_WriteCMD(0x15);
            EPD_W21_WriteDATA(0x00);

            EPD_W21_WriteCMD(0x50);     // VCOM AND DATA INTERVAL SETTING
            EPD_W21_WriteDATA(0x11);
            EPD_W21_WriteDATA(0x07);

            EPD_W21_WriteCMD(0x60);     // TCON SETTING
            EPD_W21_WriteDATA(0x22);
        }

        void fullUpdate()
        {
            EPD_Init_RBW();
            this->writeBufferToScreen();
            EPD_W21_WriteCMD(0x12);     // DISPLAY REFRESH
            driver_delay_xms(1);        // The delay here is necessary, 200uS at least
            lcd_chkstatus();
            EPD_sleep();
        }

        void drawPixel(int x, int y, uint8_t color)
        {
            size_t pos = y * 800 + x;
            size_t index = pos / 8;
            size_t bit = 7 - (pos % 8);

            // Color encoding: 0b[BW][RW]
            // White=0b10, Black=0b00, Red=0b01

            if (color & 0b10) {  // BW plane bit
                this->bufferBW[index] |= (1 << bit);
            } else {
                this->bufferBW[index] &= ~(1 << bit);
            }

            if (color & 0b01) {  // RW plane bit
                this->bufferRW[index] |= (1 << bit);
            } else {
                this->bufferRW[index] &= ~(1 << bit);
            }
        }
    };
}
