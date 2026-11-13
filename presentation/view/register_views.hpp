#ifndef register_views_HPP_
#define register_views_HPP_

#include "libsdb/register_metadata.hpp"
#include <string_view>

namespace sdb {
// virtual interfaces for the products of the factory
class register_view {
public:
  virtual ~register_view() = default;
  virtual void print_registers(const sdb::register_info &info,
                               const std::string_view value) = 0;
};

// contrete products
class minimal_register_view : public register_view {
public:
  void print_registers(const sdb::register_info &info,
                       const std::string_view value) override;
};

class json_register_view : public register_view {
public:
  void print_registers(const sdb::register_info &info,
                       const std::string_view value) override;
};

} // namespace sdb

#endif // register_views_HPP_
