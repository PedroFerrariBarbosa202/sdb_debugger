#include "register_views.hpp"
#include "libsdb/register_metadata.hpp"
#include <fmt/printf.h>

void sdb::minimal_register_view::print_registers(const sdb::register_info &info,
                                                 const std::string_view value) {
  fmt::print("{}:\t{}\n", info.name, value);
}

void sdb::json_register_view::print_registers(const sdb::register_info &info,
                                              const std::string_view value) {

  fmt::print("{{\n"
             "    \"{}\": \"{}\"\n"
             "}}\n",
             info.name, value);
}
