#ifndef SDB_PROCESS_HPP
#define SDB_PROCESS_HPP

#include <filesystem>
#include <memory>
#include <optional>
#include <sys/types.h>

#include <libsdb/registers.hpp>

namespace sdb {
enum class process_state {
  stopped,
  running,
  exited,
  terminated,
};

class stop_reason {
public:
  stop_reason(int wait_status);

  process_state reason() const;
  std::uint8_t info() const;

private:
  process_state reason_;
  std::uint8_t info_;
};

class process {
public:
  /*disable constructors*/
  process() = delete;
  process(const process &) = delete;
  process &operator=(const process &) = delete;

  ~process();

  static std::unique_ptr<process>
  launch(std::filesystem::path path, bool debug = true,
         std::optional<int> stdout_replacement = std::nullopt);

  static std::unique_ptr<process> attach(pid_t pid);

  void resume();
  stop_reason wait_on_signal();

  pid_t pid() const { return pid_; }
  process_state state() const { return state_; }

  /*register-related code*/
  registers &get_registers() { return *registers_; }
  const registers &get_registers() const { return *registers_; }

  void write_user_area(std::size_t offset, std::uint64_t data);

  void write_fprs(const user_fpregs_struct &fprs);
  void write_gprs(const user_regs_struct &gprs);

  virt_addr get_pc() const {
    return virt_addr{
        get_registers().read_by_id_as<std::uint64_t>(register_id::rip)};
  }

private:
  pid_t pid_ = 0;
  bool terminate_on_end_ = true;
  bool is_attached_ = true;
  process_state state_ = process_state::stopped;
  std::unique_ptr<registers> registers_;

  /*private constructor*/
  process(pid_t pid, bool terminate_on_end, bool is_attached)
      : pid_(pid), terminate_on_end_(terminate_on_end),
        is_attached_(is_attached), registers_(new registers(*this)) {}

  void read_all_registers();
};
} // namespace sdb

#endif
