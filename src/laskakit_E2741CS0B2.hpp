
#include "laskakit_epaper.hpp"
#include "laskakit_E2741FS081.hpp"


namespace LaskaKit {
namespace Epaper {


class E2741CS0B2 : public E2741FS081
{
public:
    static constexpr ColorType COLORTYPE = ColorType::BW;
    static constexpr const char* NAME = "E2741CS0B2";

    E2741CS0B2(const EPDBusSettings& settings)
        : E2741FS081(settings)
    {}
};

}  // ns EPaper
}  // ns LaskaKit
