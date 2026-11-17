#ifndef breakpoint_views_HPP_
#define breakpoint_views_HPP_

#include <libsdb/process.hpp>

namespace sdb {
// virtual interfaces for the products of the factory
class breakpoint_view {
public:
  virtual ~breakpoint_view() = default;

  virtual void print_list_breakpoints(sdb::process &process) = 0;
};

// contrete products
class minimal_breakpoint_view : public breakpoint_view {
public:
  void print_list_breakpoints(sdb::process &process) override;
};

class json_breakpoint_view : public breakpoint_view {
public:
  void print_list_breakpoints(sdb::process &process) override;
};

} // namespace sdb

#endif // breakpoint_views_HPP_
