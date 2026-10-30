#include "sdb_cli.hpp"
#include <iostream>

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

void sdb::sdb_cli::print_stop_reason(const sdb::process &process,
                                     sdb::stop_reason reason) {
  std::cout << "Process " << process.pid() << ' ';

  switch (reason.reason()) {
  case sdb::process_state::exited: {
    std::cout << "eited with status " << static_cast<int>(reason.info());
  } break;

  case sdb::process_state::terminated: {
    std::cout << "terminated with status " << static_cast<int>(reason.info());
  } break;

  case sdb::process_state::stopped: {
    std::cout << "eited with status " << static_cast<int>(reason.info());
  } break;
  }
}

void sdb::sdb_cli::print_help(const std::vector<std::string> &args) {
  if (args.size() == 1) {
    std::cerr << R"(Available commands:
      continue  - Resume the process
      registerister  - Commands for operating on registers
)";
  } else if (is_prefix(args[1], "register")) {
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
