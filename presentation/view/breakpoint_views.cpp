#include "breakpoint_views.hpp"
#include <fmt/printf.h>

void sdb::minimal_breakpoint_view::print_list_breakpoints(
    sdb::process &process) {
  fmt::print("Current breakpoints:\n");
  process.breakpoint_sites().for_each([](auto &site) {
    fmt::print("{}: address = {:#x}, {}\n", site.id(), site.address().addr(),
               site.is_enabled() ? "enabled" : "disabled");
  });
}

void sdb::json_breakpoint_view::print_list_breakpoints(sdb::process &process) {
  fmt::print("{{\n  \"breakpoints\": [\n");

  bool first = true;

  process.breakpoint_sites().for_each([&](auto &site) {
    if (!first)
      fmt::print(",\n");

    fmt::print("    {{\n"
               "      \"id\": {},\n"
               "      \"address\": \"{:#x}\",\n"
               "      \"enabled\": {}\n"
               "    }}",
               site.id(), site.address().addr(),
               site.is_enabled() ? "true" : "false");

    first = false;
  });

  fmt::print("\n  ]\n}}\n");
}
