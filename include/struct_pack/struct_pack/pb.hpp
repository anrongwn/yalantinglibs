/*
 * Copyright (c) 2022, Alibaba Group Holding Limited;
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <ostream>

#include "struct_pack/struct_pack/reflection.h"
namespace struct_pack {
namespace pb {
template <typename T>
class varint {
 public:
  varint() = default;
  varint(T t) : val(t) {}
  operator T() const { return val; }
  friend std::ostream& operator<<(std::ostream& os, const varint& varint) {
    os << varint.val;
    return os;
  }

 private:
  T val;
};
using varint32_t = varint<int32_t>;
using varuint32_t = varint<uint32_t>;
using varint64_t = varint<int64_t>;
using varuint64_t = varint<uint64_t>;
// clang-format off
template <typename T>
concept VARINT =
       std::same_as<T, int8_t>
    || std::same_as<T, uint8_t>
    || std::same_as<T, int16_t>
    || std::same_as<T, uint16_t>
    || std::same_as<T, varint32_t>
    || std::same_as<T, varuint32_t>
    || std::same_as<T, varint64_t>
    || std::same_as<T, varuint64_t>
    || std::same_as<T, bool>
    || std::is_enum_v<T>
;
template <typename T>
concept I64 =
    std::same_as<T, int64_t>
    || std::same_as<T, uint64_t>
    || std::same_as<T, double>
;
template <typename T>
concept LEN = std::same_as<T, std::string>
    || std::is_class_v<T>
    || detail::container<T>;
template <typename T>
concept I32 = std::same_as<T, int64_t>
    || std::same_as<T, uint64_t>
    || std::same_as<T, double>
;
// clang-format on
std::size_t calculate_needed_size() { return 0; }
template <typename T, typename... Args>
std::size_t calculate_needed_size(const T& t, const Args&... args);
template <typename T>
std::size_t calculate_one_size(const T& t);
template <typename T>
std::size_t get_needed_size(const T& t) {
  static_assert(std::is_aggregate_v<T>);
  std::size_t ret = 0;
  detail::visit_members(t, [&ret](auto&&... args) {
    ret += calculate_needed_size(args...);
  });
  return ret;
}
std::size_t calculate_varint_size(uint64_t t) {
  std::size_t ret = 0;
  do {
    ret++;
    t >>= 7;
  } while (t != 0);
  return ret;
}
template <typename T>
std::size_t calculate_one_size(const T& t) {
  if constexpr (VARINT<T>) {
    return 1 + calculate_varint_size(t);
  }
  else if constexpr (LEN<T>) {
    if constexpr (std::same_as<T, std::string>) {
      return 1 + calculate_varint_size(t.size()) + t.size();
    }
    else if constexpr (std::is_class_v<T>) {
      std::size_t ret = 0;
      detail::visit_members(t, [&ret](auto&&... args) {
        ret += calculate_needed_size(args...);
      });
      // tag, len, payload
      return 1 + 1 + ret;
    }
    else {
      static_assert(!sizeof(T), "ERROR type");
      return 0;
    }
  }
  else if constexpr (I64<T>) {
    return 1 + 4;
  }
  else if constexpr (I32<T>) {
    return 1 + 2;
  }
  else {
    static_assert(!sizeof(T), "ERROR type");
    return 0;
  }
}
template <typename T, typename... Args>
std::size_t calculate_needed_size(const T& t, const Args&... args) {
  auto size = calculate_one_size(t);
  return size + calculate_needed_size(args...);
}
}  // namespace pb
}  // namespace struct_pack