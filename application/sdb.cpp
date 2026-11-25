#include <cstdlib>
#include <cstring>
#include <libsdb/error.hpp>
#include <libsdb/process.hpp>

#include "../presentation/sdb_cli.hpp"
#include <libsdb/register_metadata.hpp>
#include <libsdb/registers.hpp>

#include <editline/readline.h>
#include <fcntl.h>
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
  // explicit constructor
  explicit sdb_debugger(sdb_cli &factory)
      : general_cli(factory.create_general_view()),
        registers_cli(factory.create_register_view()),
        errors_cli(factory.create_error_view()) {}

  // functions to call contrete views
  void print_message(const std::string_view str) {
    general_cli->print_message(str);
  }

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

  // attaches to a process or launches a new one
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

  // main loop of the program
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

  // handles a call to register read
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
      std::vector<register_info> regs_info =
          sdb::register_metadata::get_register_infos();
      for (auto &info : regs_info) {
        auto should_print =
            (args.size() == 3 or info.type == sdb::register_type::gpr) and
            info.name != "orig_rax";

        if (!should_print)
          continue;

        // gets the register to read and prints it to screen
        auto value = process.get_registers().read(info);
        this->print_register(info, std::visit(format, value));
      }
    }
    // else read register by name
    else if (args.size() == 3) {
      try {
        auto info = sdb::register_metadata::register_info_by_name(args[2]);
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

  // handles a call to register write
  void handle_register_write(sdb::process &process,
                             const std::vector<std::string> &args) {
    if (args.size() != 4) {
      this->print_help({"help", "register"});
      return;
    }

    try {
      // get register and parse the value to write into it
      auto info = sdb::register_metadata::register_info_by_name(args[2]);
      auto value = registers::parse_register_value(info, args[3]);
      process.get_registers().write(info, value);
    } catch (sdb::error &err) {
      this->print_error(err);
      return;
    }
  }

  // delegates work to the correct handler
  void handle_register_command(sdb::process &process,
                               const std::vector<std::string> &args) {
    if (args.size() < 2) {
      // prints the help instructions
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

  // sets the STDERR_FILENO to a different file descriptor
  void handle_set_ofstream(const std::vector<std::string> &args) {
    if (args.size() < 2) {
      this->print_help({"help"});
      return;
    }

    int fd = open(args[1].c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    sdb_cli::set_ofstream(fd);
    close(fd);

    print_message("Output stream changed to " + args[1] + "\n");
  }

  // restore the output stream to STDERR_FILENO
  void handle_restore_ofstream() {
    sdb_cli::restore_ofstream();
    print_message("Output stream restores to STDERR_FILENO\n");
  }

  // delegates work to the correct command handler
  void handle_command(std::unique_ptr<sdb::process> &process,
                      std::string_view line) {
    auto args = sdb_cli::split(line, ' ');
    auto command = args[0];

    if (sdb_cli::is_prefix(command, "quit")) {
      // quit program
      exit(0);
    }

    else if (sdb_cli::is_prefix(command, "continue")) {
      // resume the process and print stop reason if necessary
      process->resume();
      auto reason = process->wait_on_signal();
      this->print_stop_reason(*process, reason);
    }

    else if (sdb_cli::is_prefix(command, "help")) {
      // print command list
      this->print_help(args);
    }

    else if (sdb_cli::is_prefix(command, "register")) {
      // handle register operations
      handle_register_command(*process, args);
    }

    else if (sdb_cli::is_prefix(command, "set_ofstream")) {
      handle_set_ofstream(args);
    }

    else if (sdb_cli::is_prefix(command, "restore_ofstream")) {
      handle_restore_ofstream();
    }

    else {
      this->print_error_message("Unknown command");
    }
  }
};
} // namespace sdb

int main(int argc, const char **argv) {
  std::unique_ptr<sdb::sdb_cli> factory =
      std::make_unique<sdb::minimal_sdb_cli>();

  /*change factory based on arguments*/
  if (argc >= 3 && sdb::sdb_cli::is_prefix(argv[2], "json_view")) {
    factory = std::make_unique<sdb::json_sdb_cli>();
  }

  sdb::sdb_debugger debugger(*factory);

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
