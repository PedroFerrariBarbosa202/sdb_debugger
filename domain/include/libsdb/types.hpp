#ifndef INCLUDElibsdbtypestypes.hpp_
#define INCLUDElibsdbtypestypes .hpp_

#include <array>
#include <cstddef>
#include <cstdint>

namespace sdb {
using byte8 = std::array<std::byte, 8>;
using byte16 = std::array<std::byte, 16>;
using byte32 = std::array<std::byte, 32>;
using byte64 = std::array<std::byte, 64>;
using byte128 = std::array<std::byte, 128>;

// essentially a wrapper for std::uint64_t
class virt_addr {
public:
  virt_addr() = default;
  explicit virt_addr(std::uint64_t addr) : addr_(addr) {}

  std::uint64_t addr() const { return addr_; }

  // repetitive operator overloading boilerplate :(
  virt_addr operator+(std::int64_t offset) const {
    return virt_addr(addr_ + offset);
  }
  virt_addr operator-(std::int64_t offset) const {
    return virt_addr(addr_ - offset);
  }
  virt_addr &operator+=(std::int64_t offset) {
    addr_ += offset;
    return *this;
  }
  virt_addr &operator-=(std::int64_t offset) {
    addr_ -= offset;
    return *this;
  }
  bool operator==(const virt_addr &other) const { return addr_ == other.addr_; }
  bool operator!=(const virt_addr &other) const { return addr_ != other.addr_; }
  bool operator<(const virt_addr &other) const { return addr_ < other.addr_; }
  bool operator<=(const virt_addr &other) const { return addr_ <= other.addr_; }
  bool operator>(const virt_addr &other) const { return addr_ > other.addr_; }
  bool operator>=(const virt_addr &other) const { return addr_ >= other.addr_; }

private:
  std::uint64_t addr_ = 0;
};

} // namespace sdb

#endif // INCLUDElibsdbtypestypes.hpp_
