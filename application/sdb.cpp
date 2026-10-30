
#include <libsdb/error.hpp>
#include <libsdb/process.hpp>

#include "../presentation/sdb_cli.hpp"

#include <algorithm>
#include <editline/readline.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

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
};
} // namespace sdb

int main(int argc, const char **argv) {
  if (argc == 1) {
    std::cerr << "No arguments given\n";
    return -1;
  }

  try {
    auto process = sdb::sdb_debugger::attach(argc, argv);
    sdb::sdb_debugger::update_loop(process);
  } catch (const sdb::error &err) {
    std::cout << err.what() << '\n';
  }
}
