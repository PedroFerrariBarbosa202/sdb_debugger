#ifndef INCLUDElibsdbregistersregisters.hpp_
#define INCLUDElibsdbregistersregisters .hpp_

#include <sys/user.h>
#include <variant>

#include <libsdb/register_metadata.hpp>
#include <libsdb/types.hpp>

namespace sdb {
class process;
class registers {
public:
  /*delete public constructors*/
  registers() = delete;
  registers(const registers &) = delete;
  registers &operator=(const registers &) = delete;

  using value =
      std::variant<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
                   std::int8_t, std::int16_t, std::int32_t, std::int64_t, float,
                   double, long double, byte8, byte16, byte32, byte64, byte128>;

  value read(const register_info &info) const;
  void write(const register_info &, value val);

  template <class T> T read_by_id_as(register_id id) const {
    return std::get<T>(read(register_info_by_id(id)));
  }

  void write_by_id(register_id id, value val) {
    write(register_info_by_id(id), val);
  }

  static sdb::registers::value parse_register_value(sdb::register_info info,
                                                    std::string_view text);

private:
  friend process;
  registers(process &proc) : proc_(&proc) {}

  template <class T> sdb::byte128 widen(const sdb::register_info &info, T t);

  user data_;
  process *proc_;
};
} // namespace sdb

#endif // INCLUDElibsdbregistersregisters.hpp_
