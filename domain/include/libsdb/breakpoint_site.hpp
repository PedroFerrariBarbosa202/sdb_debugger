#ifndef breakpoint_site_HPP_
#define breakpoint_site_HPP_

#include <cstddef>
#include <cstdint>
#include <libsdb/types.hpp>

namespace sdb {
class process;

class breakpoint_site {
public:
  breakpoint_site() = default;
  breakpoint_site(const breakpoint_site &) = delete;
  breakpoint_site &operator=(const breakpoint_site &) = delete;

  using id_type = std::int32_t;
  id_type id() const { return id_; }

  void enable();
  void disable();

  bool is_enabled() const { return is_enabled_; }
  virt_addr address() const { return address_; }

  bool in_range(virt_addr low, virt_addr high) const {
    return low <= address_ and high > address_;
  }

private:
  friend process;
  breakpoint_site(process &proc, virt_addr address);

  static id_type get_next_id();

  id_type id_;
  process *process_;
  virt_addr address_;
  bool is_enabled_;
  std::byte saved_data_;
};
} // namespace sdb

#endif // breakpoint_site_HPP_
