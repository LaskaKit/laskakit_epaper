
#include "laskakit_epaper.hpp"
#include "laskakit_E2741FS081.hpp"


namespace LaskaKit {
namespace Epaper {


class E2741CS0B2 : public E2741FS081
{
public:
    static constexpr ColorType COLORTYPE = ColorType::BW;
};

}  // ns EPaper
}  // ns LaskaKit
