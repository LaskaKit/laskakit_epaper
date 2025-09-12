#pragma once

#include <laskakit_epaper.hpp>

#include "raw.hpp"


namespace LaskaKit::Epaper {
    class GDEY075T7 : public Display
    {
    private:
        uint8_t pin_power;
        uint8_t buffer[800 * 480];

    public:
        GDEY075T7(int pin_ss, int pin_dc, int pin_rst, int pin_busy, int pin_power)
            : pin_power(pin_power)
        {
            pinMode(pin_power, OUTPUT);
            this->on();

            delay(500);
            pinMode(pin_busy, INPUT);
            pinMode(pin_rst, OUTPUT);
            pinMode(pin_dc, OUTPUT);
            pinMode(pin_ss, OUTPUT);

            //SPI
            SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0)); 
            SPI.begin (12, -1, 11, pin_ss);
            this->setupBuffer();
        }

        uint width()
        {
            return 800;
        }

        uint height()
        {
            return 480;
        }

        void on()
        {
            digitalWrite(this->pin_power, HIGH);
        }

        void off()
        {
            digitalWrite(this->pin_power, LOW);
        }

        void setupBuffer()
        {
            uint8_t* buffer = this->buffer;
            for (int row = 0; row < 480; row++) {
                for (int col = 0; col < 800; col++) {
                    int index = row * 800 + col;
                    
                    if (row < 120) {
                        // Top quarter: White
                        buffer[index] = 0x11;
                    } else if (row < 240) {
                        // Second quarter: Light Gray
                        buffer[index] = 0x10;
                    } else if (row < 360) {
                        // Third quarter: Dark Gray
                        buffer[index] = 0x01;
                    } else {
                        // Bottom quarter: Black
                        buffer[index] = 0x00;
                    }
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
            uint8_t* buffer = this->buffer;
            // Old data (0x10)
            EPD_W21_WriteCMD(0x10);
            for (size_t i = 0; i < 48000; i++) {
                uint8_t temp3 = 0;
                
                // Process 8 pixels per byte
                for (int bit = 0; bit < 8; bit++) {
                    uint32_t pixel_index = i * 8 + bit;
                    
                    // Check bounds
                    if (pixel_index >= (480 * 800)) break;
                    
                    uint8_t pixel_value = buffer[pixel_index];
                    
                    temp3 <<= 1; // Shift previous bits left
                    
                    // Extract old data bit (bit 4 of your 2-bit format)
                    if (pixel_value & 0x10) {
                        temp3 |= 1; // Set bit if 0x10 or 0x11
                    }
                    // else bit remains 0 for 0x00 and 0x01
                }
                EPD_W21_WriteDATA(temp3);
            }
            
            // New data (0x13)
            EPD_W21_WriteCMD(0x13);
            for (size_t i = 0; i < 48000; i++) {
                uint8_t temp3 = 0;
                
                // Process 8 pixels per byte
                for (int bit = 0; bit < 8; bit++) {
                    uint32_t pixel_index = i * 8 + bit;
                    
                    // Check bounds
                    if (pixel_index >= (480 * 800)) break;
                    
                    uint8_t pixel_value = buffer[pixel_index];
                    
                    temp3 <<= 1; // Shift previous bits left
                    
                    // Extract new data bit (bit 0 of your 2-bit format)
                    if (pixel_value & 0x01) {
                        temp3 |= 1; // Set bit if 0x01 or 0x11
                    }
                    // else bit remains 0 for 0x00 and 0x10
                }
                EPD_W21_WriteDATA(temp3);
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
            this->buffer[pos] = color;
        }

        ~GDEY075T7()
        {
            SPI.end();
            this->off();
        }
    };
}
