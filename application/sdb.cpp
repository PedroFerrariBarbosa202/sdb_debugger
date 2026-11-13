#include <libsdb/error.hpp>
#include <libsdb/process.hpp>

#include "../presentation/sdb_cli.hpp"
#include <libsdb/register_metadata.hpp>
#include <libsdb/registers.hpp>

#include <editline/readline.h>
#include <iostream>
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
  explicit sdb_debugger(sdb_cli &factory)
      : general_cli(factory.create_general_view()),
        registers_cli(factory.create_register_view()),
        errors_cli(factory.create_error_view()) {}

  // functions to call contrete views
  void print_help(const std::vector<std::string> &args) {
    general_cli->print_help(args);
  }

  void print_register(const sdb::register_info &info,
                      const std::string_view value) {
    registers_cli->print_registers(info, value);
  }

  void print_stop_reason(const sdb::process &process, sdb::stop_reason reason) {
    errors_cli->print_stop_reason(process, reason);
  }

  void print_error_message(const std::string_view str) {
    errors_cli->print_error_message(str);
  }

  void print_error(const error &error) { errors_cli->print_error(error); }

  // class methods
  std::unique_ptr<sdb::process> attach(int argc, const char **argv) {
    /*passing PID*/
    if (argc == 3 && argv[1] == std::string_view("-p")) {
      pid_t pid = std::atoi(argv[2]);
      return sdb::process::attach(pid);
    } else { /*passing program name*/
      const char *program_path = argv[1];
      return sdb::process::launch(program_path);
    }
  }

  void handle_command(std::unique_ptr<sdb::process> &process,
                      std::string_view line) {
    auto args = sdb_cli::split(line, ' ');
    auto command = args[0];

    if (sdb_cli::is_prefix(command, "continue")) {
      process->resume();
      auto reason = process->wait_on_signal();
      this->print_stop_reason(*process, reason);
    } else if (sdb_cli::is_prefix(command, "help")) {
      this->print_help(args);
    } else if (sdb_cli::is_prefix(command, "register")) {
      handle_register_command(*process, args);
    } else {
      this->print_error_message("Unknown command");
    }
  }

  void update_loop(std::unique_ptr<sdb::process> &process) {
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
          this->print_error(err);
        }
      }
    }
  }

private:
  std::unique_ptr<general_view> general_cli;
  std::unique_ptr<register_view> registers_cli;
  std::unique_ptr<error_view> errors_cli;

  void handle_register_read(sdb::process &process,
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
        this->print_register(info, std::visit(format, value));
      }
    }
    // else read register by name
    else if (args.size() == 3) {
      try {
        auto info = sdb::register_info_by_name(args[2]);
        auto value = process.get_registers().read(info);

        this->print_register(info, std::visit(format, value));
      } catch (sdb::error &err) {
        this->print_error(err);
      }
    }
    // else print register help message
    else {
      this->print_help({"help", "register"});
    }
  }

  void handle_register_write(sdb::process &process,
                             const std::vector<std::string> &args) {
    if (args.size() != 4) {
      this->print_help({"help", "register"});
      return;
    }

    try {
      auto info = sdb::register_info_by_name(args[2]);
      auto value = registers::parse_register_value(info, args[3]);
      process.get_registers().write(info, value);
    } catch (sdb::error &err) {
      this->print_error(err);
      return;
    }
  }

  void handle_register_command(sdb::process &process,
                               const std::vector<std::string> &args) {
    if (args.size() < 2) {
      this->print_help({"help", "register"});
      return;
    }

    if (sdb_cli::is_prefix(args[1], "read")) {
      handle_register_read(process, args);
    } else if (sdb_cli::is_prefix(args[1], "write")) {
      handle_register_write(process, args);
    } else {
      this->print_help({"help", "register"});
    }
  }
};
} // namespace sdb

int main(int argc, const char **argv) {
  sdb::json_sdb_cli factory;
  sdb::sdb_debugger debugger(factory);

  if (argc == 1) {
    debugger.print_error_message("No arguments given");
    return -1;
  }

  try {
    // attach to user-defined process
    auto process = debugger.attach(argc, argv);

    // enter main process loop
    debugger.update_loop(process);
  } catch (const sdb::error &err) {
    debugger.print_error(err);
  }
}
