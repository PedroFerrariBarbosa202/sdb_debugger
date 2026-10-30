#include <libsdb/process.hpp>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <libsdb/error.hpp>
#include <libsdb/pipe.hpp>

namespace {
void exit_with_perror(sdb::pipe &channel, std::string const &prefix) {
  auto message = prefix + ": " + std::strerror(errno);
  channel.write(reinterpret_cast<std::byte *>(message.data()), message.size());
  exit(-1);
}
} // namespace

sdb::process_state sdb::stop_reason::reason() const { return reason_; }

std::uint8_t sdb::stop_reason::info() const { return info_; }

sdb::stop_reason::stop_reason(int wait_status) {
  if (WIFEXITED(wait_status)) {
    reason_ = process_state::exited;
    info_ = WEXITSTATUS(wait_status);
  } else if (WIFSIGNALED(wait_status)) {
    reason_ = process_state::terminated;
    info_ = WTERMSIG(wait_status);
  } else if (WIFSTOPPED(wait_status)) {
    reason_ = process_state::stopped;
    info_ = WSTOPSIG(wait_status);
  }
}

sdb::stop_reason sdb::process::wait_on_signal() {
  int wait_status;
  int options = 0;

  if (waitpid(pid_, &wait_status, options) < 0) {
    error::send_errno("waitpit failed");
  }
  stop_reason reason(wait_status);
  state_ = reason.reason();

  if (is_attached_ and state_ == process_state::stopped) {
    read_all_registers();
  }

  return reason;
}

std::unique_ptr<sdb::process>
sdb::process::launch(std::filesystem::path path, bool debug,
                     std::optional<int> stdout_replacement) {
  pipe channel(/*close_on_exec*/ true);

  pid_t pid;
  if ((pid = fork()) < 0) {
    error::send_errno("fork failed");
  }

  /*is child*/
  if (pid == 0) {
    channel.close_read();
    if (debug and ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
      exit_with_perror(channel, "Tracing failed");
    }
    if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
      exit_with_perror(channel, "exec failed");
    }
    if (stdout_replacement) {
      if (dup2(*stdout_replacement, STDOUT_FILENO) < 0) {
        exit_with_perror(channel, "stdout replacement failed");
      }
    }
  }
  /*is parent*/
  channel.close_write();
  auto data = channel.read();
  channel.close_read();

  if (data.size() > 0) {
    waitpid(pid, nullptr, 0);
    auto chars = reinterpret_cast<char *>(data.data());
    error::send(std::string(chars, chars + data.size()));
  }

  std::unique_ptr<sdb::process> proc(new process(pid, true, debug));

  if (debug) {
    proc->wait_on_signal();
  }

  return proc;
}

std::unique_ptr<sdb::process> sdb::process::attach(pid_t pid) {
  if (pid == 0) {
    error::send_errno("invalid PID");
  }
  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
    error::send_errno("attach failed");
  }

  std::unique_ptr<sdb::process> proc(new process(pid, true, true));
  proc->wait_on_signal();

  return proc;
}

void sdb::process::resume() {
  if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0) {
    error::send_errno("could not resume");
  }
  state_ = process_state::running;
}

sdb::process::~process() {
  if (pid_ != 0) {
    int status;
    if (is_attached_) {
      if (state_ == process_state::running) {
        kill(pid_, SIGSTOP);
        waitpid(pid_, &status, 0);
      }

      ptrace(PTRACE_DETACH, pid_, nullptr);
      kill(pid_, SIGCONT);
    }
    if (terminate_on_end_) {
      kill(pid_, SIGKILL);
      waitpid(pid_, &status, 0);
    }
  }
}

/*related to registers*/
void sdb::process::read_all_registers() {
  if (ptrace(PTRACE_GETREGS, pid_, nullptr, &get_registers().data_.regs) < 0) {
    error::send_errno("Could not read GPR registers");
  }

  if (ptrace(PTRACE_GETFPREGS, pid_, nullptr, &get_registers().data_.i387) <
      0) {
    error::send_errno("Could not read FPR registers");
  }

  for (int i = 0; i < 8; i++) {
    auto id = static_cast<int>(register_id::dr0) + i;
    auto info = register_info_by_id(static_cast<register_id>(id));

    errno = 0;
    std::int64_t data = ptrace(PTRACE_PEEKUSER, pid_, info.offset, nullptr);
    if (errno != 0)
      error::send_errno("could not read debug register");

    get_registers().data_.u_debugreg[i] = data;
  }
}

void sdb::process::write_user_area(std::size_t offset, std::uint64_t data) {
  if (ptrace(PTRACE_POKEUSER, pid_, offset, data) < 0) {
    error::send_errno("Could not write to user area");
  }
}

void sdb::process::write_fprs(const user_fpregs_struct &fprs) {
  if (ptrace(PTRACE_SETFPREGS, pid_, nullptr, &fprs) < 0) {
    error::send_errno("Could not write floating point registers");
  }
}

void sdb::process::write_gprs(const user_regs_struct &gprs) {
  if (ptrace(PTRACE_SETREGS, pid_, nullptr, &gprs) < 0) {
    error::send_errno("Could not write to heneral purpouse registers");
  }
}
