#ifndef parse_HPP_
#define parse_HPP_

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

#include <libsdb/error.hpp>

namespace sdb {
class parser {
public:
  template <class I>
  static std::optional<I> to_integral(std::string_view sv, int base = 10) {
    if constexpr (std::is_same_v<I, std::byte>) {
      auto value = to_integral<std::uint8_t>(sv, base);

      if (value)
        return static_cast<std::byte>(*value);

      return std::nullopt;
    } else {
      auto begin = sv.begin();

      if (base == 16 && sv.size() > 1 && begin[0] == '0' && begin[1] == 'x') {
        begin += 2;
      }

      I ret;
      auto result = std::from_chars(begin, sv.end(), ret, base);

      if (result.ec != std::errc{} || result.ptr != sv.end())
        return std::nullopt;

      return ret;
    }
  }
  template <class F> static std::optional<F> to_float(std::string_view sv) {
    F ret;
    auto result = std::from_chars(sv.begin(), sv.end(), ret);

    if (result.ptr != sv.end()) {
      return std::nullopt;
    }
    return ret;
  }

  template <std::size_t N> static auto parse_vector(std::string_view text) {
    auto invalid = [] { sdb::error::send("invalid format"); };

    std::array<std::byte, N> bytes;
    const char *c = text.data();

    if (*c++ != '[')
      invalid();

    for (auto i = 0; i < N - 1; i++) {
      bytes[i] = to_integral<std::byte>({c, 4}, 16).value();
      c += 4;
      if (*c++ != ',')
        invalid();
    }

    bytes[N - 1] = to_integral<std::byte>({c, 4}, 16).value();
    c += 4;

    if (*c++ != ']')
      invalid();

    if (c != text.end())
      invalid();

    return bytes;
  }
};
} // namespace sdb

#endif // parse_HPP_
