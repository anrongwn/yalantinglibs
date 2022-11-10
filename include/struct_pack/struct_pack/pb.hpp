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
#include <cassert>
#include <ostream>

#include "struct_pack/struct_pack/reflection.h"
#include "struct_pack/struct_pack/struct_pack_impl.hpp"
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
  static_assert(std::is_class_v<T>);
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
    else if constexpr (detail::map_container<T> || detail::container<T>) {
      using value_type = typename T::value_type;
      std::size_t sz = 0;
      for (auto&& i : t) {
        if constexpr (VARINT<value_type>) {
          sz += calculate_varint_size(i);
        }
        else {
          sz += calculate_one_size(i);
        }
      }
      return 1 + calculate_varint_size(t.size()) + sz;
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
using wire_type_t = uint8_t;
template <typename T>
wire_type_t get_wire_type() {
  if constexpr (VARINT<T>) {
    return 0;
  }
  else if constexpr (I64<T>) {
    return 1;
  }
  else if constexpr (LEN<T>) {
    return 2;
  }
  else if constexpr (I32<T>) {
    return 5;
  }
  else {
    static_assert(!sizeof(T), "SGROUP and EGROUP are deprecated");
  }
}
template <typename T>
constexpr std::size_t first_field_number = 1;
template <detail::struct_pack_byte Byte>
class packer {
 public:
  packer(Byte* data, std::size_t max) : data_(data), max_(max) {}
  template <typename T>
  void serialize(const T& t) {
    constexpr auto Count = detail::member_count<T>();
    serialize(t, std::make_index_sequence<Count>());
  }

 private:
  template <typename T, std::size_t... I>
  void serialize(const T& t, std::index_sequence<I...>) {
    constexpr auto Num = first_field_number<T>;
    (serialize(get_field<T, I + Num>(t), I + Num), ...);
  }
  template <typename T, std::size_t FieldNumber>
  const auto& get_field(const T& t) {
    constexpr auto Count = detail::member_count<T>();
    constexpr std::size_t Index = FieldNumber - first_field_number<T>;
    static_assert(Index >= 0 && Index <= Count);
    if constexpr (Count == 1) {
      const auto& [a1] = t;
      return std::get<Index>(std::forward_as_tuple(a1));
    }
    else if constexpr (Count == 2) {
      const auto& [a1, a2] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2));
    }
    else if constexpr (Count == 3) {
      const auto& [a1, a2, a3] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2, a3));
    }
    else {
      static_assert(!sizeof(T), "wait for add hard code");
    }
  }
  template <typename T>
  void serialize(const T& t, std::size_t field_number) {
    auto tag = (field_number << 3) | get_wire_type<T>();
    assert(pos_ < max_);
    data_[pos_++] = tag;
    if constexpr (VARINT<T>) {
      serialize_varint(t);
    }
    else if constexpr (I64<T> || I32<T>) {
      std::memcpy(data_ + pos_, &t, sizeof(T));
      pos_ += sizeof(T);
    }
    else if constexpr (LEN<T>) {
      if constexpr (std::same_as<T, std::string>) {
        serialize_varint(t.size());
        assert(pos_ + t.size() <= max_);
        std::memcpy(data_ + pos_, t.data(), t.size());
        pos_ += t.size();
      }
      else if constexpr (detail::map_container<T> || detail::container<T>) {
        using value_type = typename T::value_type;
        auto sz_pos = pos_;
        // risk to data len > 1byte
        pos_++;
        std::size_t sz = 0;
        for (auto&& e : t) {
          if constexpr (VARINT<value_type>) {
            sz += calculate_varint_size(e);
            serialize_varint(e);
          }
          else {
            sz += get_needed_size(e);
            serialize(e);
          }
        }
        pos_ = sz_pos;
        auto new_pos = pos_;
        serialize_varint(sz);
        pos_ = new_pos;
      }
      else {
        auto sz = get_needed_size(t);
        serialize_varint(sz);
        serialize(t);
      }
    }
    else {
      static_assert(!sizeof(T), "SGROUP and EGROUP are deprecated");
    }
  }
  void serialize_varint(uint64_t t) {
    do {
      assert(pos_ < max_);
      data_[pos_++] = 0b1000'0000 | uint8_t(t);
      t >>= 7;
    } while (t != 0);
    assert(pos_ > 0);
    data_[pos_ - 1] = uint8_t(data_[pos_ - 1]) & 0b0111'1111;
  }

 private:
  Byte* data_;
  std::size_t pos_ = 0;
  std::size_t max_;
};
template <detail::struct_pack_buffer Buffer = std::vector<char>,
          typename... Args>
[[nodiscard]] STRUCT_PACK_INLINE Buffer serialize(const Args&... args) {
  static_assert(sizeof...(args) == 1);
  Buffer buffer;
  auto size = get_needed_size(args...);
  buffer.resize(size);
  packer o(buffer.data(), size);
  o.serialize(args...);
  return buffer;
}
}  // namespace pb
}  // namespace struct_pack