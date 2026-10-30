#ifndef sdb_cli_HPP_
#define sdb_cli_HPP_

#include <libsdb/process.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace sdb {
class sdb_cli {
public:
  static void print_stop_reason(const sdb::process &process,
                                sdb::stop_reason reason);

  static void print_help(const std::vector<std::string> &args);

  static bool is_prefix(std::string_view str, std::string_view of);
  static std::vector<std::string> split(std::string_view str, char delimiter);
};
} // namespace sdb

#endif // sdb_cli_HPP_
