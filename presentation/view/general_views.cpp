#include "general_views.hpp"
#include "../sdb_cli.hpp"
#include "fmt/printf.h"
#include <iostream>

void sdb::minimal_general_view::print_help(
    const std::vector<std::string> &args) {
  if (args.size() == 1) {
    std::cerr << R"(Available commands:
      quit      - Exit the program
      continue  - Resume the process
      register  - Commands for operating on registers
)";
  } else if (sdb::sdb_cli::is_prefix(args[1], "register")) {
    std::cerr << R"(Available commands:
      read
      read <register>
      read all
      write <register> <value>
)";
  } else {
    std::cerr << "No help available for that command\n";
  }
}

void sdb::json_general_view::print_help(const std::vector<std::string> &args) {

  if (args.size() == 1) {
    fmt::print(R"(
{{
    "commands": {{
        "quit"    : "Exit the program"
        "continue": "Resume the process",
        "register": "Commands for operating on registers"
    }}
}}
)");
  } else if (sdb::sdb_cli::is_prefix(args[1], "register")) {
    fmt::print(R"(
{{
    "register": {{
        "read": "Read register values",
        "read <register>": "Read a specific register",
        "read all": "Read all registers",
        "write <register> <value>": "Write a value to a register"
    }}
}}
)");
  } else {
    fmt::print(R"(
{{
    "error": "No help available for that command"
}}
)");
  }
}
