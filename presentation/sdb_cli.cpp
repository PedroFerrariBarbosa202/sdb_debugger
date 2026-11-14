#include "sdb_cli.hpp"
#include <fmt/printf.h>

std::vector<std::string> sdb::sdb_cli::split(std::string_view str,
                                             char delimiter) {
  std::vector<std::string> out{};
  std::stringstream ss{std::string{str}};
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    out.push_back(item);
  }

  return out;
}

bool sdb::sdb_cli::is_prefix(std::string_view str, std::string_view of) {
  if (str.size() > of.size())
    return false;
  return std::equal(str.begin(), str.end(), of.begin());
}
