#ifndef sdb_cli_HPP_
#define sdb_cli_HPP_

#include <libsdb/error.hpp>
#include <libsdb/process.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace sdb {
// virtual interfaces for the products of the factory
class general_view {
public:
  virtual ~general_view() = default;
  virtual void print_help(const std::vector<std::string> &args) = 0;
};

class register_view {
public:
  virtual ~register_view() = default;
  virtual void print_registers(const std::string_view name,
                               const std::string_view value) = 0;
};

class error_view {
public:
  virtual ~error_view() = default;
  virtual void print_stop_reason(const sdb::process &process,
                                 sdb::stop_reason reason) = 0;
  virtual void print_error(const error &error) = 0;
};

// contrete products
class minimal_general_view : public general_view {
public:
  void print_help(const std::vector<std::string> &args) override;
};

class minimal_register_view : public register_view {
public:
  void print_registers(const std::string_view name,
                       const std::string_view value) override;
};

class minimal_error_view : public error_view {
public:
  void print_stop_reason(const sdb::process &process,
                         sdb::stop_reason reason) override;
  void print_error(const error &error) override;
};

class json_general_view : public general_view {
public:
  void print_help(const std::vector<std::string> &args) override;
};

class json_register_view : public register_view {
public:
  void print_registers(const std::string_view name,
                       const std::string_view value) override;
};

class json_error_view : public error_view {
public:
  void print_stop_reason(const sdb::process &process,
                         sdb::stop_reason reason) override;
  void print_error(const error &error) override;
};

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
