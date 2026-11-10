
#include <libsdb/error.hpp>
#include <libsdb/process.hpp>

#include "../presentation/sdb_cli.hpp"
#include <libsdb/register_metadata.hpp>
#include <libsdb/registers.hpp>

#include <algorithm>
#include <editline/readline.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unistd.h>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

namespace sdb {
class sdb_debugger {
public:
  static void handle_register_command(sdb::process &process,
                                      const std::vector<std::string> &args) {
    if (args.size() < 2) {
      sdb_cli::print_help({"help", "register"});
      return;
    }

    if (sdb_cli::is_prefix(args[1], "read")) {
      handle_register_read(process, args);
    } else if (sdb_cli::is_prefix(args[1], "write")) {
      handle_register_write(process, args);
    } else {
      sdb_cli::print_help({"help", "register"});
    }
  }

  static std::unique_ptr<sdb::process> attach(int argc, const char **argv) {
    /*passing PID*/
    if (argc == 3 && argv[1] == std::string_view("-p")) {
      pid_t pid = std::atoi(argv[2]);
      return sdb::process::attach(pid);
    } else { /*passing program name*/
      const char *program_path = argv[1];
      return sdb::process::launch(program_path);
    }
  }

  static void handle_command(std::unique_ptr<sdb::process> &process,
                             std::string_view line) {
    auto args = sdb_cli::split(line, ' ');
    auto command = args[0];

    if (sdb_cli::is_prefix(command, "continue")) {
      process->resume();
      auto reason = process->wait_on_signal();
      sdb_cli::print_stop_reason(*process, reason);
    } else if (sdb_cli::is_prefix(command, "help")) {
      sdb_cli::print_help(args);
    } else if (sdb_cli::is_prefix(command, "register")) {
      handle_register_command(*process, args);
    } else {
      std::cerr << "unknown command\n";
    }
  }

  static void update_loop(std::unique_ptr<sdb::process> &process) {
    char *line = nullptr;
    while ((line = readline("sdb> ")) != nullptr) {
      std::string line_str;

      if (line == std::string_view("")) {
        free(line);
        if (history_length > 0) {
          line_str = history_list()[history_length - 1]->line;
        }
      } else {
        line_str = line;
        add_history(line);
        free(line);
      }

      if (!line_str.empty()) {
        try {
          handle_command(process, line_str);
        } catch (const sdb::error &err) {
          std::cout << err.what() << '\n';
        }
      }
    }
  }

private:
  static void handle_register_read(sdb::process &process,
                                   const std::vector<std::string> &args) {
    // lambda to format strings to format different data types
    auto format = [](auto t) {
      if constexpr (std::is_floating_point_v<decltype(t)>) {
        return fmt::format("{}", t);
      } else if constexpr (std::is_integral_v<decltype(t)>) {
        return fmt::format("{:#0{}x}", t, sizeof(t) * 2 + 2);
      } else {
        return fmt::format("[{:#04x}]", fmt::join(t, ","));
      }
    };

    // if user passes 2 args or "all" as args[2], it should print all gpr regs
    if (args.size() == 2 or (args.size() == 3 and args[2] == "all")) {
      for (auto &info : sdb::g_register_infos) {
        auto should_print =
            (args.size() == 3 or info.type == sdb::register_type::gpr) and
            info.name != "orig_rax";

        if (!should_print)
          continue;

        auto value = process.get_registers().read(info);
        fmt::print("{}:\t{}\n", info.name,
                   std::visit(format, value)); // TODO pass to sdb_cli
      }
    }
    // else read register by name
    else if (args.size() == 3) {
      try {
        auto info = sdb::register_info_by_name(args[2]);
        auto value = process.get_registers().read(info);
        fmt::print("{}:\t{}\n", info.name,
                   std::visit(format, value)); // TODO pass to sdb_cli
      } catch (sdb::error &err) {
        std::cerr << "No such registe\n";
      }
    }
    // else print register help message
    else {
      sdb_cli::print_help({"help", "register"});
    }
  }

  static void handle_register_write(sdb::process &process,
                                    const std::vector<std::string> &args) {
    if (args.size() != 4) {
      sdb_cli::print_help({"help", "register"});
      return;
    }

    try {
      auto info = sdb::register_info_by_name(args[2]);
      auto value = registers::parse_register_value(info, args[3]);
      process.get_registers().write(info, value);
    } catch (sdb::error &err) {
      std::cerr << err.what() << '\n';
      return;
    }
  }
};
} // namespace sdb

int main(int argc, const char **argv) {
  if (argc == 1) {
    std::cerr << "No arguments given\n";
    return -1;
  }

  try {
    // attach to user-defined process
    auto process = sdb::sdb_debugger::attach(argc, argv);

    // enter main process loop
    sdb::sdb_debugger::update_loop(process);
  } catch (const sdb::error &err) {
    std::cout << err.what() << '\n';
  }
}
