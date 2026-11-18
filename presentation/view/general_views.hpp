#ifndef general_views_HPP_
#define general_views_HPP_

#include <string>
#include <vector>

namespace sdb {
// virtual interfaces for the product of the factory
class general_view {
public:
  virtual ~general_view() = default;
  virtual void print_message(const std::string_view message) = 0;
  virtual void print_help(const std::vector<std::string> &args) = 0;
};

// contrete products
class minimal_general_view : public general_view {
public:
  void print_message(const std::string_view message) override;
  void print_help(const std::vector<std::string> &args) override;
};

class json_general_view : public general_view {
public:
  void print_message(const std::string_view message) override;
  void print_help(const std::vector<std::string> &args) override;
};

} // namespace sdb

#endif // general_views_HPP_
