#pragma once

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
};

class EPDBus {
private:
    int8_t cs;
    int8_t dc;
    int8_t busy;
    int8_t reset;

    static EPDBus* instance;

    EPDBus(int8_t cs, int8_t dc, int8_t busy, int8_t reset)
        : cs(cs), dc(dc), busy(busy), reset(reset)
    {}

    EPDBus(const EPDBusSettings& settings)
        : cs(settings.cs), dc(settings.dc), busy(settings.busy), reset(settings.reset)
    {}

public:
    static void Begin(int8_t sck, int8_t mosi, int8_t cs, int8_t dc, int8_t busy, int8_t reset)
    {
        if (instance != nullptr) {
            return;
        }
        instance = new EPDBus(cs, dc, busy, reset);
        SPI.begin(sck, -1, mosi);
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
        EPDBus::Begin(settings.sck, settings.mosi, settings.cs, settings.dc, settings.busy, settings.reset);
    }

    static void End()
    {
        if (instance == nullptr) {
            return;
        }
        SPI.end();
        delete instance;
    }

    static void BeginTransaction()
    {
        SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
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
        while (digitalRead(instance->busy)) { delay(10); }
    }

    static void BusyWaitInv()
    {
        while (!digitalRead(instance->busy)) { delay(10); }
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

