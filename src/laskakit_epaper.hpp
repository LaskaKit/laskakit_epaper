#pragma once

#include <Arduino.h>
// #include <Adafruit_GFX.h>
#include <memory>


namespace LaskaKit::Epaper {
    enum class ColorType {
        BW,  // black and white
        G4,  // four shades of gray
        G8,  // eight shades of gray
        RBW, // red, black, white
        YBW, // yellow, black, white
        C4,  // four colors (black, white, red, yellow)
        C7,  // seven colors
    };

    enum class Color {
        BLACK,
        WHITE,
        YELLOW,
        RED,
        GRAY_0,
        GRAY_1,
        GRAY_2,
        GRAY_3,
        GRAY_4,
        GRAY_5,
        GRAY_6,
        GRAY_7,
    };

    enum class DisplayRotation
    {
        NO_ROTATION,
        RIGHT_90,
        LEFT_90,
        RIGHT_180,
        LEFT_180,
        RIGHT_270,
        LEFT_270
    };

    // common interface for all displays
    class Display
    {
    public:
        // virtual size_t width() = 0;
        // virtual size_t height() = 0;
        // virtual void on() = 0;
        // virtual void off() = 0;
        virtual void fullUpdate() = 0;
        // virtual void setRotation(DisplayRotation rot) = 0;
        // virtual void fillScreen(int color) = 0;
        virtual void drawPixel(int x, int y, uint8_t color) = 0;
        // virtual void drawText(int x, int y, const String& text, uint8_t color) = 0;
        // virtual void drawCenteredText(int x, int y, const String& text, uint8_t color) = 0;
        // virtual void fillRect(int x, int y, int width, int heitht) = 0;
        virtual ~Display() {}
    };

    // class DisplayGFX : public Adafruit_GFX
    // {
    // private:
    //     std::unique_ptr<Display> display;
    // public:
    //     DisplayGFX(std::unique_ptr<Display>& display)
    //         : Adafruit_GFX(display->width(), display->height()),
    //           display(std::move(display))
    //     {
    //     }

    //     void drawPixel(int16_t x, int16_t y, uint16_t color)
    //     {
    //         this->display->drawPixel(x, y, color);
    //     };

    //     void fullUpdate()
    //     {
    //         this->display->fullUpdate();
    //     }
    // };
}
