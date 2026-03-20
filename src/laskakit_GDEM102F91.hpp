#pragma once

#include <GxEPD2_4C.h>

#include "epdbus.hpp"
#include "laskakit_GDEY116F51.hpp"

#define ENABLE_GxEPD2_GFX 0

namespace LaskaKit::Epaper {

class GDEM102F91 : public GDEY116F51 {
public:
    GDEM102F91(const EPDBusSettings& settings)
        : GDEY116F51(settings)
    {}
};

}
