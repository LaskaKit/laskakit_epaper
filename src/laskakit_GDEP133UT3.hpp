#pragma once

#include <epdiy.h>

#include "laskakit_epaper.hpp"
#include "epdbus.hpp"


namespace LaskaKit::Epaper {

    class GDEP133UT3 {
    public:
        static constexpr size_t WIDTH = 1600;
        static constexpr size_t HEIGHT = 1200;
        static constexpr ColorType COLORTYPE = ColorType::G16;
        static constexpr const char* NAME = "GDEP133UT3";
    private:
        EpdiyHighlevelState hl;
        uint8_t* fb;

    public:
        GDEP133UT3(const EPDBusSettings& settings) {}

        bool init()
        {
            epd_init(&epd_board_v7, &ED133UT2, EPD_LUT_64K);
            epd_set_vcom(300);
            hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);
            fb = epd_hl_get_framebuffer(&hl);
            return true;
        }

        void fullUpdate()
        {
            epd_poweron();
            epd_clear();
            epd_hl_update_screen(&hl, MODE_EPDIY_WHITE_TO_GL16, epd_ambient_temperature());
            epd_poweroff();
        }

        void drawPixel(int16_t x, int16_t y, uint8_t color)
        {
            if (color > 0xF) {
                return;
            }
            // epdiy uses 0->255
            epd_draw_pixel(x, y, color * 17, fb);
        }
    };
} // LaskaKit::Epaper
