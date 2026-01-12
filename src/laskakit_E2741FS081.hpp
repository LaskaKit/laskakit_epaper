#pragma once

#include <cstdint>
#include <memory>

#include <Arduino.h>
#include <SPI.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"


namespace LaskaKit::Epaper {

class E2741FS081
{
public:
    static constexpr uint16_t WIDTH = 480;
    static constexpr uint16_t HEIGHT = 800;
    static constexpr ColorType COLORTYPE = ColorType::RBW;
    static constexpr const char* NAME = "E2741FS081";

private:
    uint8_t* frame1;
    uint8_t* frame2;


public:
    E2741FS081(const EPDBusSettings& settings)
    {
        // Allocate the buffers
        const size_t frameSize = this->WIDTH * this->HEIGHT / 8;
        this->frame1 = (uint8_t*)calloc(frameSize, sizeof(uint8_t));
        this->frame2 = (uint8_t*)calloc(frameSize, sizeof(uint8_t));
        if (this->frame1 == nullptr) {
            Serial.println("malloc frame1 failed");
        }
        if (this->frame2 == nullptr) {
            Serial.println("malloc frame2 failed");
        }

        // clear
        memset(frame1, 0x00, 48000);
        memset(frame2, 0x00, 48000);

        // init pins
        pinMode(PIN_PWR, OUTPUT);
        pinMode(PIN_EPD_CS, OUTPUT);
        pinMode(PIN_EPD_DC, OUTPUT);
        pinMode(PIN_EPD_RST, OUTPUT);
        pinMode(PIN_EPD_BUSY, INPUT);

        // init SPI
        SPI.begin(PIN_EPD_SCL, -1, PIN_EPD_SDA, PIN_EPD_CS);
    }

    ~E2741FS081()
    {
        if (this->frame1) {
            free(this->frame1);
        }
        if (this->frame2) {
            free(this->frame2);
        }
    }

    void on()
    {
        // Power on
        digitalWrite(PIN_PWR, HIGH);
        delay(500);

        // Reset display
        digitalWrite(PIN_EPD_RST, HIGH);
        delay(20);
        digitalWrite(PIN_EPD_RST, LOW);
        delay(200);
        digitalWrite(PIN_EPD_RST, HIGH);
        delay(50);
        digitalWrite(PIN_EPD_CS, HIGH);
    }

    void setupBuffer()
    {

        for (int row = 0; row < this->HEIGHT; row++) {
            for (int col = 0; col < this->WIDTH; col += 8) {
                int index = (row * this->WIDTH + col) / 8;
                uint8_t tmpFrame1 = 0;
                uint8_t tmpFrame2 = 0;

                for (int bit = 0; bit < 8; bit++) {
                    tmpFrame1 <<= 1;
                    tmpFrame2 <<= 1;
                    if (row < 100) {
                        // First part: Red
                        tmpFrame1 |= 0x0;
                        tmpFrame2 |= 0x1;
                    } else if (row < 200) {
                        // Second part: Black
                        tmpFrame1 |= 0x1;
                        tmpFrame2 |= 0x0;
                    } else {
                        // Third part: White
                        tmpFrame1 |= 0x0;
                        tmpFrame2 |= 0x0;
                    }
                }
                this->frame1[index] = tmpFrame1;
                this->frame2[index] = tmpFrame2;
            }
        }
    }

    // | Frame1 | Frame2 | Color |
    // | ------ | ------ | ----- |
    // |    1   |     1  |    ?? |
    // |    1   |     0  | black |
    // |    0   |     1  |   red |
    // |    0   |     0  | white |
    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        // size_t pos = y * 800 + x;
        size_t pos = y * this->WIDTH + x;
        size_t index = pos / 8;
        size_t shift = 7 - (pos % 8);
        uint8_t mask = 0b1 << shift;

        if (index > 47950) {
            // printf("index: %lu\n", index);
            return;
        }
        // printf("%d %d %lu %lu %u\n", x, y, index, shift, color);

        if (color == RGB565::WHITE) {
            this->frame1[index] &= ~mask;
            this->frame2[index] &= ~mask;
        }

        if (color == RGB565::BLACK) {
            this->frame1[index] |= mask;
            this->frame2[index] &= ~mask;
        }

        if (color == RGB565::RED) {
            this->frame1[index] &= ~mask;
            this->frame2[index] |= mask;
        }

        // if (color & 0b10) {
        //     this->frame1[index] |= (0b1 << shift);
        // } else {
        //     this->frame1[index] &= ~(0xFF & (0b1 << shift));
        // }

        // if (color & 0b01) {
        //     this->frame2[index] |= (0b1 << shift);
        // } else {
        //     this->frame2[index] &= ~(0xFF & (0b1 << shift));
        // }
        // this->buffer[pos] = color;
        // printf("%lu\n", shift);
    }

    void fullUpdate()
    {
        // MAX SPI freq = 5 MHz (from datasheet)
        SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));

        this->on();
        // Send hardcoded configuration values from working example
        uint8_t data1[] = {0x00, 0x3b, 0x00, 0x00, 0x1f, 0x03};
        this->sendIndexData(0x13, data1, 6);    // DUW

        uint8_t data2[] = {0x00, 0x3b, 0x00, 0xc9};
        this->sendIndexData(0x90, data2, 4);    // DRFW

        uint8_t data3[] = {0x3b, 0x00, 0x14};
        this->sendIndexData(0x12, data3, 3);    // RAM_RW

        // Send first frame (black pixels)
        Serial.println("Sending first frame (black pixels)...");
        this->sendIndexData(0x10, this->frame1, 48000);

        // Send RAM_RW again
        this->sendIndexData(0x12, data3, 3);    // RAM_RW

        // Send second frame (red pixels)
        Serial.println("Sending second frame (red pixels)...");
        this->sendIndexData(0x11, this->frame2, 48000);

        Serial.println("Image data sent successfully.");

        // Initialize COG
        this->cogInitialization();

        // DC/DC soft-start
        this->dcDcSoftStart();

        Serial.println("Data send and COG init finished.");

        // Display refresh and power down
        this->displayRefreshAndPowerDown();

        Serial.println("=== Complete Display Update Finished ===\n");

        SPI.endTransaction();
    }

public:
    void sendIndexData(uint8_t index, const uint8_t* data, uint32_t len)
    {
        digitalWrite(PIN_EPD_DC, LOW);
        digitalWrite(PIN_EPD_CS, LOW);
        SPI.transfer(index);

        digitalWrite(PIN_EPD_CS, HIGH);
        digitalWrite(PIN_EPD_DC, HIGH);
        digitalWrite(PIN_EPD_CS, LOW);
        for (int i = 0; i < len; i++) {
            SPI.transfer(data[i]);
        }
        // SPI.transferBytes(data, nullptr, len);
        digitalWrite(PIN_EPD_CS, HIGH);
    }

    void cogInitialization()
    {
        Serial.println("=== Starting COG Initialization ===");

        // Initial COG setup using hardcoded values from working example
        uint8_t data4[] = {0x7d};
        sendIndexData(0x05, data4, 1);
        delay(1);

        uint8_t data5[] = {0x00};
        sendIndexData(0x05, data5, 1);
        delay(1);

        uint8_t data6[] = {0x3f};
        sendIndexData(0xc2, data6, 1);
        delay(1);

        uint8_t data7[] = {0x00};
        sendIndexData(0xd8, data7, 1);    // MS_SYNC

        uint8_t data8[] = {0x00};
        sendIndexData(0xd6, data8, 1);    // BVSS

        uint8_t data9[] = {0x10};
        sendIndexData(0xa7, data9, 1);
        delay(1);

        sendIndexData(0xa7, data5, 1);
        delay(1);

        uint8_t data10[] = {0x00, 0x01};
        sendIndexData(0x03, data10, 2);    // OSC
        sendIndexData(0x44, data5, 1);
        uint8_t data11[] = {0x80};
        sendIndexData(0x45, data11, 1);
        sendIndexData(0xa7, data9, 1);
        delay(1);

        sendIndexData(0xa7, data7, 1);
        delay(1);

        uint8_t data12[] = {0x06};
        sendIndexData(0x44, data12, 1);
        uint8_t data13[] = {0x82};
        sendIndexData(0x45, data13, 1);    // Temperature 0x82@25C 0xFE@87
        sendIndexData(0xa7, data9, 1);
        delay(1);

        sendIndexData(0xa7, data7, 1);
        delay(1);

        uint8_t data14[] = {0x25};
        sendIndexData(0x60, data14, 1);    // TCON
        uint8_t data15[] = {0x00};
        sendIndexData(0x61, data15, 1);    // STV_DIR
        uint8_t data16[] = {0x00};
        sendIndexData(0x01, data16, 1);    // DCTL
        uint8_t data17[] = {0x00};
        sendIndexData(0x02, data17, 1);    // VCOM

        Serial.println("COG initialization complete.");
    }

    void dcDcSoftStart()
    {
        Serial.println("=== Starting DC/DC Soft-Start ===");

        // DCDC soft-start sequence from working example
        uint8_t Index51_data[] = {0x50, 0x01, 0x0a, 0x01};
        sendIndexData(0x51, &Index51_data[0], 2); // 0x51-0x50,0x01
        uint8_t Index09_data[] = {0x1f, 0x9f, 0x7f, 0xff};

        // Stage 1
        for(int value = 1; value <= 4; value++) {
            sendIndexData(0x09, &Index09_data[0], 1);
            Index51_data[1] = value;
            sendIndexData(0x51, &Index51_data[0], 2);
            sendIndexData(0x09, &Index09_data[1], 1);
            delay(1);
        }

        // Stage 2
        for(int value = 1; value <= 10; value++) {
            sendIndexData(0x09, &Index09_data[0], 1);
            Index51_data[3] = value;
            sendIndexData(0x51, &Index51_data[2], 2);
            sendIndexData(0x09, &Index09_data[1], 1);
            delay(1);
        }

        // Stage 3
        for(int value = 3; value <= 10; value++) {
            sendIndexData(0x09, &Index09_data[2], 1);
            Index51_data[3] = value;
            sendIndexData(0x51, &Index51_data[2], 2);
            sendIndexData(0x09, &Index09_data[3], 1);
            delay(1);
        }

        // Stage 4
        for(int value = 9; value >= 2; value--) {
            sendIndexData(0x09, &Index09_data[2], 1);
            Index51_data[2] = value;
            Index51_data[3] = 0x0A;
            sendIndexData(0x51, &Index51_data[2], 2);
            sendIndexData(0x09, &Index09_data[3], 1);
            delay(1);
        }
        sendIndexData(0x09, &Index09_data[3], 1);
        delay(1);

        Serial.println("DC/DC soft-start complete.");
    }

    void displayRefreshAndPowerDown()
    {
        Serial.println("=== Starting Display Refresh ===");

        // Wait for BUSY pin to go high
        while(digitalRead(PIN_EPD_BUSY) != HIGH) {
            delay(10);
        }

        // Send display refresh command
        uint8_t data18[] = {0x3c};
        sendIndexData(0x15, data18, 1);    // Display Refresh
        delay(1);

        Serial.println("Display refresh command sent. Waiting for completion...");

        int start = millis();
        // Wait for refresh to complete
        while(digitalRead(PIN_EPD_BUSY) != HIGH) {
            delay(100);
        }

        Serial.println("Display refresh complete. Starting power down...");
        Serial.printf("Refresh duration: %f seconds\n", (millis() - start) / 1000.0);

        // DCDC off sequence
        uint8_t data19[] = {0x7f};
        sendIndexData(0x09, data19, 1);
        uint8_t data20[] = {0x7d};
        sendIndexData(0x05, data20, 1);
        uint8_t data5[] = {0x00};
        sendIndexData(0x09, data5, 1);
        delay(200);

        while(digitalRead(PIN_EPD_BUSY) != HIGH) {
            delay(10);
        }

        // Set all pins to low
        digitalWrite(PIN_EPD_DC, LOW);
        digitalWrite(PIN_EPD_CS, HIGH);
        digitalWrite(PIN_EPD_SDA, LOW);
        digitalWrite(PIN_EPD_SCL, LOW);
        digitalWrite(PIN_EPD_RST, LOW);
        digitalWrite(PIN_PWR, LOW);

        Serial.println("Display power down complete.");
    }
};

}
