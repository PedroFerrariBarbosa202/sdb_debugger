#ifndef INCLUDElibsdbtypestypes.hpp_
#define INCLUDElibsdbtypestypes.hpp_

#include <array>
#include <cstddef>

namespace sdb{
  using byte64 = std::array<std::byte, 64>;
  using byte128 = std::array<std::byte, 128>;
}

#endif  // INCLUDElibsdbtypestypes.hpp_
