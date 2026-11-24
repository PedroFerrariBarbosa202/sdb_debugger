#ifndef SDB_PIPE_HPP
#define SDB_PIPE_HPP

#include <cstddef>
#include <vector>

namespace sdb {
/*wrapper for pipe2*/
class pipe {
public:
  explicit pipe(bool close_on_exec);
  ~pipe();

  /*gets the read end of the pipe*/
  int get_read() const { return fds_[read_fd]; }
  /*gets the write end of the pipe*/
  int get_write() const { return fds_[write_fd]; }
  int release_read();
  int release_write();
  void close_read();
  void close_write();

  /*reads bytes from pipe*/
  std::vector<std::byte> read();
  /*writes bytes to pipe*/
  void write(std::byte *from, std::size_t bytes);

private:
  /*file descriptor constants for reading and writing to pipe*/
  static constexpr unsigned read_fd = 0;
  static constexpr unsigned write_fd = 1;

  /*pipe's file descriptors*/
  int fds_[2];
};
} // namespace sdb

#endif
