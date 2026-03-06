#pragma once

#include <cstdint>

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
        EPDBus::Begin(settings);

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
        size_t pos = y * this->WIDTH + x;
        size_t index = pos / 8;
        size_t shift = 7 - (pos % 8);
        uint8_t mask = 0b1 << shift;

        if (index > 47950) {
            return;
        }

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
    }

    void fullUpdate()
    {
        Serial.println("Starting a full update.");
        // MAX SPI freq = 5 MHz (from datasheet)
        EPDBus::BeginTransaction();

        EPDBus::DelayMs(200);
        EPDBus::Reset(HIGH);
        EPDBus::DelayMs(20);
        EPDBus::Reset(LOW);
        EPDBus::DelayMs(200);
        EPDBus::Reset(HIGH);
        EPDBus::DelayMs(50);

        // Send hardcoded configuration values from working example
        EPDBus::WriteCmdData(0x13, {0x00, 0x3b, 0x00, 0x00, 0x1f, 0x03});  // DUW
        EPDBus::WriteCmdData(0x90, {0x00, 0x3b, 0x00, 0xc9});  // DRFW

        // Send first frame (black pixels)
        EPDBus::WriteCmdData(0x12, {0x3b, 0x00, 0x14});  // RAM_RW
        EPDBus::_WriteCmdData(0x10, this->frame1, 48000);

        // Send second frame (red pixels)
        EPDBus::WriteCmdData(0x12, {0x3b, 0x00, 0x14});  // RAM_RW
        EPDBus::_WriteCmdData(0x11, this->frame2, 48000);

        // Initialize COG
        this->cogInitialization();

        // DC/DC soft-start
        this->dcDcSoftStart();

        Serial.println("Data send and COG init finished.");

        // Display refresh and power down
        this->displayRefreshAndPowerDown();

        Serial.println("=== Complete Display Update Finished ===\n");

        EPDBus::EndTransaction();
    }

private:

    void cogInitialization()
    {
        Serial.println("COG init start.");

        // Initial COG setup using hardcoded values from working example
        EPDBus::WriteCmdData(0x05, {0x7d});
        EPDBus::DelayMs(1);
        EPDBus::WriteCmdData(0x05, {0x00});
        EPDBus::DelayMs(1);
        EPDBus::WriteCmdData(0xc2, {0x3f});
        EPDBus::DelayMs(1);
        EPDBus::WriteCmdData(0xd8, {0x00});  // MS_SYNC
        EPDBus::WriteCmdData(0xd6, {0x00});  // BVSS
        EPDBus::WriteCmdData(0xa7, {0x10});
        EPDBus::DelayMs(1);
        EPDBus::WriteCmdData(0xa7, {0x00});
        EPDBus::DelayMs(1);
        EPDBus::WriteCmdData(0x03, {0x00, 0x01});
        EPDBus::WriteCmdData(0x44, {0x00});
        EPDBus::WriteCmdData(0x45, {0x80});
        EPDBus::WriteCmdData(0xa7, {0x10});
        EPDBus::DelayMs(1);
        EPDBus::WriteCmdData(0xa7, {0x00});
        EPDBus::DelayMs(1);
        EPDBus::WriteCmdData(0x44, {0x06});
        EPDBus::WriteCmdData(0x45, {0x82});
        EPDBus::WriteCmdData(0xa7, {0x10});
        EPDBus::DelayMs(1);
        EPDBus::WriteCmdData(0xa7, {0x00});
        EPDBus::DelayMs(1);
        EPDBus::WriteCmdData(0x60, {0x25});
        EPDBus::WriteCmdData(0x61, {0x00});
        EPDBus::WriteCmdData(0x01, {0x00});  // DCTL
        EPDBus::WriteCmdData(0x02, {0x00});  // VCOM

        Serial.println("COG init end.");
    }

    void dcDcSoftStart()
    {
        Serial.println("=== Starting DC/DC Soft-Start ===");
        // DCDC soft-start sequence from working example
        uint8_t Index51_data[] = {0x50, 0x01, 0x0a, 0x01};
        uint8_t Index09_data[] = {0x1f, 0x9f, 0x7f, 0xff};

        EPDBus::_WriteCmdData(0x51, Index51_data, 2);

        // Stage 1
        for(int value = 1; value <= 4; value++) {
            EPDBus::_WriteCmdData(0x09, Index09_data, 1);
            Index51_data[1] = value;
            EPDBus::_WriteCmdData(0x51, Index51_data, 2);
            EPDBus::_WriteCmdData(0x09, &Index09_data[1], 1);
            EPDBus::DelayMs(1);
        }

        // Stage 2
        for(int value = 1; value <= 10; value++) {
            EPDBus::_WriteCmdData(0x09, Index09_data, 1);
            Index51_data[3] = value;
            EPDBus::_WriteCmdData(0x51, &Index51_data[2], 2);
            EPDBus::_WriteCmdData(0x09, &Index09_data[1], 1);
            EPDBus::DelayMs(1);
        }

        // Stage 3
        for(int value = 3; value <= 10; value++) {
            EPDBus::_WriteCmdData(0x09, &Index09_data[2], 1);
            Index51_data[3] = value;
            EPDBus::_WriteCmdData(0x51, &Index51_data[2], 2);
            EPDBus::_WriteCmdData(0x09, &Index09_data[3], 1);
            EPDBus::DelayMs(1);
        }

        // Stage 4
        for(int value = 9; value >= 2; value--) {
            EPDBus::_WriteCmdData(0x09, &Index09_data[2], 1);
            Index51_data[2] = value;
            Index51_data[3] = 0x0A;
            EPDBus::_WriteCmdData(0x51, &Index51_data[2], 2);
            EPDBus::_WriteCmdData(0x09, &Index09_data[3], 1);
            EPDBus::DelayMs(1);
        }
        EPDBus::_WriteCmdData(0x09, &Index09_data[3], 1);
        EPDBus::DelayMs(1);
        Serial.println("DC/DC soft-start complete.");
    }

    void displayRefreshAndPowerDown()
    {
        Serial.println("=== Starting Display Refresh ===");
        // Wait for BUSY pin to go high
        EPDBus::BusyWaitInv();
        // EPDBus::BusyWait();
        // Send display refresh command
        EPDBus::WriteCmdData(0x15, {0x3c});
        EPDBus::DelayMs(1);
        EPDBus::BusyWaitInv();
        // EPDBus::BusyWait();

        // DCDC off sequence
        EPDBus::WriteCmdData(0x09, {0x7f});
        EPDBus::WriteCmdData(0x05, {0x7d});
        EPDBus::WriteCmdData(0x09, {0x00});
        EPDBus::DelayMs(200);
        EPDBus::BusyWaitInv();
        // EPDBus::BusyWait();
        Serial.println("Display power down complete.");
    }
};

}
