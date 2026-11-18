#include "general_views.hpp"
#include "../sdb_cli.hpp"
#include "fmt/printf.h"
#include <iostream>

void sdb::minimal_general_view::print_message(const std::string_view message) {
  std::cout << message << "\n";
}

void sdb::minimal_general_view::print_help(
    const std::vector<std::string> &args) {
  if (args.size() == 1) {
    std::cout << R"(Available commands:
      quit      - Exit the program
      continue  - Resume the process
      register  - Commands for operating on registers
      set_ofstream - Sets the output file. Follow the command with the name of the file desired
      restore_ofstream - Restores the original file descriptor, writing text in the terminal

)";
  } else if (sdb::sdb_cli::is_prefix(args[1], "register")) {
    std::cout << R"(Available commands:
      read
      read <register>
      read all
      write <register> <value>
)";
  } else {
    std::cout << "No help available for that command\n";
  }
}

void sdb::json_general_view::print_message(const std::string_view message) {
  fmt::print(R"({{
    "message": "{}"
}}
)",
             message);
}

void sdb::json_general_view::print_help(const std::vector<std::string> &args) {

  if (args.size() == 1) {
    fmt::print(R"(
{{
    "commands": {{
        "quit"    : "Exit the program"
        "continue": "Resume the process",
        "register": "Commands for operating on registers"
        "set_ofstream": "Sets the output file. Follow the command with the name of the file desired"
        "restore_ofstream": "Restores the original file descriptor, writing text in the terminal"
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
