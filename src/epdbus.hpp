#pragma once

#include "driver/gpio.h"
#include "esp32-hal-gpio.h"
#include "esp_sleep.h"
#include "hal/gpio_types.h"
#include <Arduino.h>
#include <SPI.h>
#include <vector>
#include <initializer_list>


namespace LaskaKit::Epaper {

struct EPDBusSettings {
    int8_t sck;
    int8_t mosi;
    int8_t cs;
    int8_t dc;
    int8_t busy;
    int8_t reset;
    int8_t pwr;
};

class EPDBus {
private:
    int8_t cs;
    int8_t dc;
    int8_t busy;
    int8_t reset;
    int8_t pwr;

    static EPDBus* instance;

    EPDBus(int8_t cs, int8_t dc, int8_t busy, int8_t reset, int8_t pwr)
        : cs(cs), dc(dc), busy(busy), reset(reset), pwr(pwr)
    {}

    EPDBus(const EPDBusSettings& settings)
        : cs(settings.cs), dc(settings.dc), busy(settings.busy), reset(settings.reset), pwr(settings.pwr)
    {}

public:
    static void Begin(int8_t sck, int8_t mosi, int8_t cs, int8_t dc, int8_t busy, int8_t reset, int8_t pwr)
    {
        if (instance != nullptr) {
            return;
        }
        if (pwr != -1) {
            pinMode(pwr, OUTPUT);
            digitalWrite(pwr, HIGH);
            delay(500);
        }


        instance = new EPDBus(cs, dc, busy, reset, pwr);
        SPI.begin(sck, -1, mosi, cs);
        pinMode(cs, OUTPUT);
        pinMode(dc, OUTPUT);
        pinMode(busy, INPUT);
        pinMode(reset, OUTPUT);
        instance->cs = cs;
        instance->dc = dc;
        instance->busy = busy;
        instance->reset = reset;
    }

    static void Begin(const EPDBusSettings& settings)
    {
        EPDBus::Begin(settings.sck, settings.mosi, settings.cs, settings.dc, settings.busy, settings.reset, settings.pwr);
    }

    static void End()
    {
        if (instance == nullptr) {
            return;
        }
        SPI.end();
        delete instance;
    }

    static void BeginTransaction(uint32_t speed = 16000000)
    {
        SPI.beginTransaction(SPISettings(speed, MSBFIRST, SPI_MODE0));
    }

    static void EndTransaction()
    {
        SPI.endTransaction();
    }

    static void Reset()
    {
        digitalWrite(instance->reset, LOW);
        delay(20);  // at least 10ms
        digitalWrite(instance->reset, HIGH);
        delay(20);
    }

    static void Reset(uint8_t val)
    {
        digitalWrite(instance->reset, val);
    }

    static void _WriteData(const uint8_t* data, size_t len)
    {
        digitalWrite(instance->cs, LOW);
        digitalWrite(instance->dc, HIGH);
        SPI.writeBytes(data, len);
        digitalWrite(instance->cs, HIGH);
    }

    template<size_t N>
    static void WriteData(const uint8_t (&data)[N])
    {
        _WriteData(data, N);
    }

    static void WriteCmd(uint8_t cmd)
    {
        digitalWrite(instance->cs, LOW);
        digitalWrite(instance->dc, LOW);
        SPI.write(cmd);
        digitalWrite(instance->cs, HIGH);
    }

    static void _WriteCmdData(uint8_t cmd, const uint8_t* data, size_t len)
    {
        WriteCmd(cmd);
        _WriteData(data, len);
    }

    static void WriteCmdData(uint8_t cmd, std::initializer_list<uint8_t> data)
    {
        std::vector<uint8_t> vec(data);
        WriteCmd(cmd);
        _WriteData(vec.data(), vec.size());
    }

    static void DelayMs(uint delayMs)
    {
        delay(delayMs);
    }

    static void BusyWait()
    {
        gpio_wakeup_enable((gpio_num_t)instance->busy, GPIO_INTR_LOW_LEVEL);
        esp_sleep_enable_gpio_wakeup();

        while (digitalRead(instance->busy)) {
            esp_light_sleep_start();
        }
        gpio_wakeup_disable((gpio_num_t)instance->busy);
    }

    static void BusyWaitInv()
    {
        gpio_wakeup_enable((gpio_num_t)instance->busy, GPIO_INTR_HIGH_LEVEL);
        esp_sleep_enable_gpio_wakeup();

        while (!digitalRead(instance->busy)) {
            esp_light_sleep_start();
        }

        gpio_wakeup_disable((gpio_num_t)instance->busy);
    }

    static void BusyPoll(uint8_t cmd)
    {
        while (true) {
            WriteCmd(cmd);
            if (digitalRead(instance->busy)) {
                break;
            }
        }
    }
};

EPDBus* EPDBus::instance = nullptr;  // definition

};  // namespace LaskaKit::Epaper
