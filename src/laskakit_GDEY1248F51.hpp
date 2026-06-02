/* LaskaKit ESPink-Dually driver for Good Display GDEY1248F51 (12.48", 1304x984, BWRY)
 *
 * Board:   LaskaKit ESPink-Dually:   https://www.laskakit.cz/laskakit-espink-dually-e-paper/
 * Display: Good Display GDEY1248F51  https://www.laskakit.cz/good-display-gdey1248z51-12-48--1304x984-epaper-cerveny-displej/
 */

#pragma once

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"

namespace LaskaKit::Epaper
{
  class GDEY1248F51
  {
  public:
    static constexpr uint16_t WIDTH  = 1304;
    static constexpr uint16_t HEIGHT = 984;
    static constexpr ColorType COLORTYPE = ColorType::BWRY;
    static constexpr const char* NAME = "GDEY1248F51";

  private:
    // EPDBusSettings covers the M1 panel + shared SPI lines + power:
    //   sck   → SCL
    //   mosi  → SDA
    //   cs    → CS_M1
    //   dc    → DC_M1  (shared with S1)
    //   busy  → BUSY_M1
    //   reset → RST_M1
    //   pwr   → power enable
    EPDBusSettings settings;

    // Additional pins for S1, M2, S2 sub-panels (ESPink-Dually board, fixed wiring)
    static constexpr int8_t PIN_CS_S1   = 18;
    static constexpr int8_t PIN_CS_M2   = 48;
    static constexpr int8_t PIN_CS_S2   = 41;
    static constexpr int8_t PIN_DC_M2   = 6;   // shared with S2
    static constexpr int8_t PIN_RST_M2  = 39;
    static constexpr int8_t PIN_BUSY_M2 = 1;

    uint8_t* frame = nullptr;

    // All four panels simultaneously — CS/DC toggled manually, byte sent via SPI
    void writeCmdAll(uint8_t cmd) {
      digitalWrite(settings.cs, LOW);
      digitalWrite(PIN_CS_S1,   LOW);
      digitalWrite(PIN_CS_M2,   LOW);
      digitalWrite(PIN_CS_S2,   LOW);
      digitalWrite(settings.dc, LOW);
      digitalWrite(PIN_DC_M2,   LOW);
      SPI.write(cmd);
      digitalWrite(PIN_CS_S2,   HIGH);
      digitalWrite(PIN_CS_M2,   HIGH);
      digitalWrite(PIN_CS_S1,   HIGH);
      digitalWrite(settings.cs, HIGH);
    }
    void writeDataAll(uint8_t d) {
      digitalWrite(settings.cs, LOW);
      digitalWrite(PIN_CS_S1,   LOW);
      digitalWrite(PIN_CS_M2,   LOW);
      digitalWrite(PIN_CS_S2,   LOW);
      digitalWrite(settings.dc, HIGH);
      digitalWrite(PIN_DC_M2,   HIGH);
      SPI.write(d);
      digitalWrite(PIN_CS_S2,   HIGH);
      digitalWrite(PIN_CS_M2,   HIGH);
      digitalWrite(PIN_CS_S1,   HIGH);
      digitalWrite(settings.cs, HIGH);
    }

    // M1+M2 simultaneously
    void writeCmdM1M2(uint8_t cmd) {
      digitalWrite(settings.cs, LOW);
      digitalWrite(PIN_CS_M2,   LOW);
      digitalWrite(settings.dc, LOW);
      digitalWrite(PIN_DC_M2,   LOW);
      SPI.write(cmd);
      digitalWrite(settings.cs, HIGH);
      digitalWrite(PIN_CS_M2,   HIGH);
    }

    // ---- display sequences ----

    void hardwareInit() {
      digitalWrite(settings.cs,    HIGH);
      digitalWrite(PIN_CS_S1,      HIGH);
      delay(10);
      digitalWrite(settings.reset, HIGH);
      digitalWrite(PIN_RST_M2,     HIGH);
      delay(100);
      digitalWrite(settings.reset, LOW);
      digitalWrite(PIN_RST_M2,     LOW);
      delay(100);
      digitalWrite(settings.reset, HIGH);
      digitalWrite(PIN_RST_M2,     HIGH);
      delay(100);
    }

    void rsdSet() {
      writeCmdAll(0xFF); writeDataAll(0xA5);
      writeCmdAll(0xCC);
      writeDataAll(0x55); writeDataAll(0xEA);
      writeDataAll(0x55); writeDataAll(0x05);
      writeCmdAll(0xFF); writeDataAll(0xE3);
      writeCmdAll(0xA0);
      EPDBus::WaitBusyHigh(settings.busy);
      EPDBus::WaitBusyHigh(PIN_BUSY_M2);
    }

    void epdInit() {
      hardwareInit();
      EPDBus::WaitBusyHigh(settings.busy);
      EPDBus::WaitBusyHigh(PIN_BUSY_M2);

      rsdSet();

      // Temperature compensation: use 25°C (0x19) as fixed value
      writeCmdAll(0xe6); writeDataAll(0x19);
      writeCmdAll(0xe0); writeDataAll(0x03);
      delay(20);

      writeCmdAll(0xA5);
      EPDBus::WaitBusyHigh(settings.busy);
      EPDBus::WaitBusyHigh(PIN_BUSY_M2);

      writeCmdAll(0xe0); writeDataAll(0x01);
      delay(10);
      EPDBus::WaitBusyHigh(settings.busy);
      EPDBus::WaitBusyHigh(PIN_BUSY_M2);

      EPDBus::WriteCmd(0x00);                               EPDBus::WriteData(0x0f, settings.cs, settings.dc); EPDBus::WriteData(0x29, settings.cs, settings.dc);
      EPDBus::WriteCmd(0x00, PIN_CS_S1, settings.dc);      EPDBus::WriteData(0x0f, PIN_CS_S1,   settings.dc); EPDBus::WriteData(0x29, settings.cs, settings.dc);
      EPDBus::WriteCmd(0x00, PIN_CS_M2, PIN_DC_M2);        EPDBus::WriteData(0x03, PIN_CS_M2,   PIN_DC_M2);   EPDBus::WriteData(0x29, settings.cs, settings.dc);
      EPDBus::WriteCmd(0x00, PIN_CS_S2, PIN_DC_M2);        EPDBus::WriteData(0x03, PIN_CS_S2,   PIN_DC_M2);   EPDBus::WriteData(0x29, settings.cs, settings.dc);

      writeCmdAll(0x01); writeDataAll(0x07); writeDataAll(0x00);
      writeCmdAll(0x03); writeDataAll(0x10); writeDataAll(0x54); writeDataAll(0x44);

      EPDBus::WriteCmd(0x06);                          EPDBus::WriteData(0xC0, settings.cs, settings.dc); EPDBus::WriteData(0xC0, settings.cs, settings.dc); EPDBus::WriteData(0xC0, settings.cs, settings.dc); EPDBus::WriteData(0x17, settings.cs, settings.dc);
      EPDBus::WriteCmd(0x06, PIN_CS_M2, PIN_DC_M2);   EPDBus::WriteData(0xC0, PIN_CS_M2,   PIN_DC_M2);   EPDBus::WriteData(0xC0, PIN_CS_M2,   PIN_DC_M2);   EPDBus::WriteData(0xC0, PIN_CS_M2,   PIN_DC_M2);   EPDBus::WriteData(0xC0, PIN_CS_M2, PIN_DC_M2);
      EPDBus::WriteCmd(0x06, PIN_CS_S1, settings.dc); EPDBus::WriteData(0xC0, PIN_CS_S1,   settings.dc); EPDBus::WriteData(0xC0, PIN_CS_S1,   settings.dc); EPDBus::WriteData(0xC0, PIN_CS_S1,   settings.dc); EPDBus::WriteData(0x17, PIN_CS_S1, settings.dc);
      EPDBus::WriteCmd(0x06, PIN_CS_S2, PIN_DC_M2);   EPDBus::WriteData(0xC0, PIN_CS_S2,   PIN_DC_M2);   EPDBus::WriteData(0xC0, PIN_CS_S2,   PIN_DC_M2);   EPDBus::WriteData(0xC0, PIN_CS_S2,   PIN_DC_M2);   EPDBus::WriteData(0xC0, PIN_CS_S2, PIN_DC_M2);

      EPDBus::WriteCmd(0x30);                          EPDBus::WriteData(0x08, settings.cs, settings.dc);
      EPDBus::WriteCmd(0x30, PIN_CS_S1, settings.dc); EPDBus::WriteData(0x08, PIN_CS_S1,   settings.dc);
      EPDBus::WriteCmd(0x30, PIN_CS_M2, PIN_DC_M2);   EPDBus::WriteData(0x08, PIN_CS_M2,   PIN_DC_M2);
      EPDBus::WriteCmd(0x30, PIN_CS_S2, PIN_DC_M2);   EPDBus::WriteData(0x08, PIN_CS_S2,   PIN_DC_M2);

      EPDBus::WriteCmd(0x82);                          EPDBus::WriteData(0x9E, settings.cs, settings.dc);
      EPDBus::WriteCmd(0x82, PIN_CS_S1, settings.dc); EPDBus::WriteData(0x9E, PIN_CS_S1,   settings.dc);
      EPDBus::WriteCmd(0x82, PIN_CS_M2, PIN_DC_M2);   EPDBus::WriteData(0x9E, PIN_CS_M2,   PIN_DC_M2);
      EPDBus::WriteCmd(0x82, PIN_CS_S2, PIN_DC_M2);   EPDBus::WriteData(0x9E, PIN_CS_S2,   PIN_DC_M2);

      writeCmdAll(0x50); writeDataAll(0x37);
      writeCmdAll(0x60); writeDataAll(0x02); writeDataAll(0x02);

      EPDBus::WriteCmd(0x61);                          EPDBus::WriteData(0x02, settings.cs, settings.dc); EPDBus::WriteData(0x88, settings.cs, settings.dc); EPDBus::WriteData(0x01, settings.cs, settings.dc); EPDBus::WriteData(0xEC, settings.cs, settings.dc);
      EPDBus::WriteCmd(0x61, PIN_CS_S1, settings.dc); EPDBus::WriteData(0x02, PIN_CS_S1,   settings.dc); EPDBus::WriteData(0x90, PIN_CS_S1,   settings.dc); EPDBus::WriteData(0x01, PIN_CS_S1,   settings.dc); EPDBus::WriteData(0xEC, PIN_CS_S1, settings.dc);
      EPDBus::WriteCmd(0x61, PIN_CS_M2, PIN_DC_M2);   EPDBus::WriteData(0x02, PIN_CS_M2,   PIN_DC_M2);   EPDBus::WriteData(0x90, PIN_CS_M2,   PIN_DC_M2);   EPDBus::WriteData(0x01, PIN_CS_M2,   PIN_DC_M2);   EPDBus::WriteData(0xEC, PIN_CS_M2, PIN_DC_M2);
      EPDBus::WriteCmd(0x61, PIN_CS_S2, PIN_DC_M2);   EPDBus::WriteData(0x02, PIN_CS_S2,   PIN_DC_M2);   EPDBus::WriteData(0x88, PIN_CS_S2,   PIN_DC_M2);   EPDBus::WriteData(0x01, PIN_CS_S2,   PIN_DC_M2);   EPDBus::WriteData(0xEC, PIN_CS_S2, PIN_DC_M2);

      writeCmdAll(0xE7); writeDataAll(0x1C);
      writeCmdAll(0xE3); writeDataAll(0x77);
      writeCmdAll(0xE9); writeDataAll(0x01);
      writeCmdAll(0xFF); writeDataAll(0xA5);
      writeCmdAll(0xEF);
      writeDataAll(1);  writeDataAll(50);
      writeDataAll(5);  writeDataAll(26);
      writeDataAll(10); writeDataAll(26);
      writeDataAll(20); writeDataAll(13);
      writeCmdAll(0xDC); writeDataAll(0x01);
      writeCmdAll(0xDD); writeDataAll(0x01);
      writeCmdAll(0xDE); writeDataAll(0x03);
      writeCmdAll(0xF9); writeDataAll(0x01);
      writeCmdAll(0xDF); writeDataAll(0x16);
      writeCmdAll(0xE8); writeDataAll(0x07);
      writeCmdAll(0xFF); writeDataAll(0xE3);
    }

    void epdUpdate() {
      writeCmdM1M2(0x04);
      EPDBus::WaitBusyHigh(settings.busy);
      EPDBus::WaitBusyHigh(PIN_BUSY_M2);
      delay(300);
      writeCmdAll(0x12); writeDataAll(0x01);
      EPDBus::WaitBusyHigh(settings.busy);
      EPDBus::WaitBusyHigh(PIN_BUSY_M2);
    }

    void epdSleep() {
      writeCmdAll(0x02); writeDataAll(0x00);
      EPDBus::WaitBusyHigh(settings.busy);
      EPDBus::WaitBusyHigh(PIN_BUSY_M2);
      writeCmdAll(0x07); writeDataAll(0xA5);
    }

    void frameDisplay() {
      unsigned int column, row;

      EPDBus::WriteCmd(0x10);
      for (column = 492; column < 984; column++)
        for (row = 0; row < 648 / 4; row++)
          EPDBus::WriteData(this->frame[row + column * 326], settings.cs, settings.dc);

      EPDBus::WriteCmd(0x10, PIN_CS_S1, settings.dc);
      for (column = 492; column < 984; column++)
        for (row = 648 / 4; row < 1304 / 4; row++)
          EPDBus::WriteData(this->frame[row + column * 326], PIN_CS_S1, settings.dc);

      EPDBus::WriteCmd(0x10, PIN_CS_M2, PIN_DC_M2);
      for (column = 0; column < 492; column++)
        for (row = 648 / 4; row < 1304 / 4; row++)
          EPDBus::WriteData(this->frame[row + column * 326], PIN_CS_M2, PIN_DC_M2);

      EPDBus::WriteCmd(0x10, PIN_CS_S2, PIN_DC_M2);
      for (column = 0; column < 492; column++)
        for (row = 0; row < 648 / 4; row++)
          EPDBus::WriteData(this->frame[row + column * 326], PIN_CS_S2, PIN_DC_M2);

      epdUpdate();
    }

  public:
    GDEY1248F51(const EPDBusSettings& settings)
      : settings(settings)
    {}

    bool init()
    {
      const size_t frameSize = WIDTH * HEIGHT / 4;
      this->frame = (uint8_t*)calloc(frameSize, sizeof(uint8_t));
      if (!this->frame) return false;

      pinMode(PIN_CS_S1,   OUTPUT);
      pinMode(PIN_CS_M2,   OUTPUT);
      pinMode(PIN_CS_S2,   OUTPUT);
      pinMode(PIN_DC_M2,   OUTPUT);
      pinMode(PIN_RST_M2,  OUTPUT);
      pinMode(PIN_BUSY_M2, INPUT);

      return true;
    }

    void drawPixel(int16_t x, int16_t y, uint8_t color)
    {
      size_t pos   = y * WIDTH + x;
      size_t index = pos / 4;
      size_t shift = 3 - (pos % 4);

      static constexpr uint8_t mapping[] = {0b00, 0b01, 0b11, 0b10};
      if (color >= 4) return;

      this->frame[index] &= ~(0b11 << (shift * 2));
      this->frame[index] |= mapping[color] << (shift * 2);
    }

    void fullUpdate()
    {
      EPDBus::BeginTransaction();
      epdInit();
      frameDisplay();
      epdSleep();
      EPDBus::EndTransaction();
    }
  };
}
