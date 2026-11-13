#ifndef error_views_HPP_
#define error_views_HPP_

#include <libsdb/process.hpp>

namespace sdb {
// virtual interfaces for the products of the factory
class error_view {
public:
  virtual ~error_view() = default;
  virtual void print_stop_reason(const sdb::process &process,
                                 sdb::stop_reason reason) = 0;
  virtual void print_error_message(const std::string_view &str) = 0;
  virtual void print_error(const error &error) = 0;
};

// contrete products
class minimal_error_view : public error_view {
public:
  void print_stop_reason(const sdb::process &process,
                         sdb::stop_reason reason) override;
  void print_error_message(const std::string_view &str) override;
  void print_error(const error &error) override;
};

class json_error_view : public error_view {
public:
  void print_stop_reason(const sdb::process &process,
                         sdb::stop_reason reason) override;
  void print_error_message(const std::string_view &str) override;
  void print_error(const error &error) override;
};

} // namespace sdb

#endif // error_views_HPP_
