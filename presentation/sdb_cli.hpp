#ifndef sdb_cli_HPP_
#define sdb_cli_HPP_

#include <libsdb/error.hpp>
#include <libsdb/process.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "view/error_views.hpp"
#include "view/general_views.hpp"
#include "view/register_views.hpp"

namespace sdb {

// abstract factory
class sdb_cli {
public:
  virtual ~sdb_cli() = default;

  virtual std::unique_ptr<general_view> create_general_view() = 0;
  virtual std::unique_ptr<register_view> create_register_view() = 0;
  virtual std::unique_ptr<error_view> create_error_view() = 0;

  static bool is_prefix(std::string_view str, std::string_view of);
  static std::vector<std::string> split(std::string_view str, char delimiter);
};

// concrete factory
class minimal_sdb_cli : public sdb_cli {
public:
  std::unique_ptr<general_view> create_general_view() override {
    return std::make_unique<minimal_general_view>();
  }

  std::unique_ptr<register_view> create_register_view() override {
    return std::make_unique<minimal_register_view>();
  }

  std::unique_ptr<error_view> create_error_view() override {
    return std::make_unique<minimal_error_view>();
  }
};

class json_sdb_cli : public sdb_cli {
public:
  std::unique_ptr<general_view> create_general_view() override {
    return std::make_unique<json_general_view>();
  }

  std::unique_ptr<register_view> create_register_view() override {
    return std::make_unique<json_register_view>();
  }

  std::unique_ptr<error_view> create_error_view() override {
    return std::make_unique<json_error_view>();
  }
};

} // namespace sdb

#endif // sdb_cli_HPP_
