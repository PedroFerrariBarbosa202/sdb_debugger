#include <fcntl.h>
#include <fmt/printf.h>
#include <unistd.h>

#include "sdb_cli.hpp"

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

void sdb::sdb_cli::set_ofstream(int fd) {
  if (original_stderr_ == -1)
    original_stderr_ = dup(STDERR_FILENO);

  dup2(fd, STDERR_FILENO);
}

void sdb::sdb_cli::restore_ofstream() {
  if (original_stderr_ != -1) {
    dup2(original_stderr_, STDERR_FILENO);
    close(original_stderr_);
    original_stderr_ = -1;
  }
}
