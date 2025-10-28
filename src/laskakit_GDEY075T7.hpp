#pragma once

#include <laskakit_epaper.hpp>

#include "raw.hpp"


namespace LaskaKit::Epaper {
    class GDEY075T7 : public Display
    {
    private:
        static const uint WIDTH = 800;
        static const uint HEIGHT = 480;
        uint8_t* bufferOld;
        uint8_t* bufferNew;

    public:
        GDEY075T7()
        {
            this->bufferOld = (uint8_t*)malloc(48000);
            this->bufferNew = (uint8_t*)malloc(48000);
            if (!this->bufferOld || !this->bufferNew) {
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

        ~GDEY075T7() {
            SPI.end();
            if (this->bufferOld) {
                free(this->bufferOld);
            }
            if (this->bufferNew) {
                free(this->bufferNew);
            }
        }

        uint width()
        {
            return this->WIDTH;
        }

        uint height()
        {
            return this->HEIGHT;
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
            // Old data (0x10)
            EPD_W21_WriteCMD(0x10);
            for (size_t i = 0; i < 48000; i++) {
                EPD_W21_WriteDATA(this->bufferOld[i]);
            }
            
            // New data (0x13)
            EPD_W21_WriteCMD(0x13);
            for (size_t i = 0; i < 48000; i++) {
                EPD_W21_WriteDATA(this->bufferNew[i]);
            }
            
            lut(); // Trigger display refresh
        }

        void fullUpdate()
        {
            EPD_init_4Gray();
            this->writeBufferToScreen();
            EPD_W21_WriteCMD(0x12);   //DISPLAY REFRESH   
            driver_delay_xms(100);    //!!!The delay here is necessary, 200uS at least!!!     
            lcd_chkstatus();
            EPD_sleep();
        }

        void drawPixel(int x, int y, uint8_t color)
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
    };
}
