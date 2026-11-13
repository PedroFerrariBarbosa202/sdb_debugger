#include "error_views.hpp"
#include <fmt/printf.h>
#include <iostream>

void sdb::minimal_error_view::print_stop_reason(const sdb::process &process,
                                                sdb::stop_reason reason) {
  std::string message;
  switch (reason.reason()) {
  case sdb::process_state::exited:
    message =
        fmt::format("exited with status {}", static_cast<int>(reason.info()));
    break;
  case sdb::process_state::terminated:
    message =
        fmt::format("terminated with signal {}", sigabbrev_np(reason.info()));
    break;
  case sdb::process_state::stopped:
    message = fmt::format("stopped with signal {} at {:#x}",
                          sigabbrev_np(reason.info()), process.get_pc().addr());
    break;
  }

  fmt::print("Process {} {}\n", process.pid(), message);
}

void sdb::minimal_error_view::print_error_message(const std::string_view &str) {
  std::cerr << "\033[31m" << str << "\033[0m\n";
}

void sdb::minimal_error_view::print_error(const sdb::error &error) {
  std::cerr << "\033[31m" << error.what() << "\033[0m\n";
}

void sdb::json_error_view::print_stop_reason(const sdb::process &process,
                                             sdb::stop_reason reason) {

  switch (reason.reason()) {
  case sdb::process_state::exited:
    fmt::print(R"({{
    "pid": {},
    "state": "exited",
    "status": {}
}}
)",
               process.pid(), static_cast<int>(reason.info()));
    break;

  case sdb::process_state::terminated:
    fmt::print(R"({{
    "pid": {},
    "state": "terminated",
    "signal": "{}"
}}
)",
               process.pid(), sigabbrev_np(reason.info()));
    break;

  case sdb::process_state::stopped:
    fmt::print(R"({{
    "pid": {},
    "state": "stopped",
    "signal": "{}",
    "pc": "{:#x}"
}}
)",
               process.pid(), sigabbrev_np(reason.info()),
               process.get_pc().addr());
    break;
  }
}

void sdb::json_error_view::print_error_message(const std::string_view &str) {
  fmt::print(stderr, R"({{
    "message": "{}"
}}
)",
             str);
}

void sdb::json_error_view::print_error(const sdb::error &error) {
  fmt::print(stderr, R"({{
    "error": "{}"
}}
)",
             error.what());
}
