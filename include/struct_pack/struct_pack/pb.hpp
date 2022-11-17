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
#include <frozen/map.h>
#include <frozen/set.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>
#include <ostream>

#include "struct_pack/struct_pack.hpp"
#include "struct_pack/struct_pack/reflection.h"
#include "struct_pack/struct_pack/struct_pack_impl.hpp"
namespace struct_pack {
namespace pb {
template <typename T>
class varint {
 public:
  using value_type = T;
  varint() = default;
  varint(T t) : val(t) {}
  operator T() const { return val; }
  auto& operator=(T t) {
    val = t;
    return *this;
  }
  auto operator<=>(const varint&) const = default;
  bool operator==(const varint<T>& t) const { return val == t.val; }
  bool operator==(T t) const { return val == t; }
  auto operator&(uint8_t mask) const {
    T new_val = val & mask;
    return varint(new_val);
  }
  template <std::unsigned_integral U>
  auto operator<<(U shift) const {
    T new_val = val << shift;
    return varint(new_val);
  }
  template <typename U>
  auto operator|=(U shift) {
    if constexpr (std::same_as<U, varint<T>>) {
      val |= shift.val;
    }
    else {
      val |= shift;
    }
    return *this;
  }
  friend std::ostream& operator<<(std::ostream& os, const varint& varint) {
    os << varint.val;
    return os;
  }

 private:
  T val;
};

template <typename T>
class sint {
 public:
  using value_type = T;
  sint() = default;
  sint(T t) : val(t) {}
  operator T() const { return val; }
  auto& operator=(T t) {
    val = t;
    return *this;
  }
  auto operator<=>(const sint<T>&) const = default;
  bool operator==(T t) const { return val == t; }
  bool operator==(const sint& t) const { return val == t.val; }
  auto operator&(uint8_t mask) const {
    T new_val = val & mask;
    return sint(new_val);
  }
  template <std::unsigned_integral U>
  auto operator<<(U shift) const {
    T new_val = val << shift;
    return sint(new_val);
  }
  friend std::ostream& operator<<(std::ostream& os, const sint& t) {
    os << t.val;
    return os;
  }

 private:
  T val;
};

template <typename T>
concept varintable =
    requires { std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>; };

template <typename T>
concept sintable =
    requires { std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>; };

using varint32_t = varint<int32_t>;
using varuint32_t = varint<uint32_t>;
using varint64_t = varint<int64_t>;
using varuint64_t = varint<uint64_t>;

using sint32_t = sint<int32_t>;
using sint64_t = sint<int64_t>;

template <typename T>
concept varintable_t =
    std::is_same_v<T, varint32_t> || std::is_same_v<T, varint64_t> ||
    std::is_same_v<T, varuint32_t> || std::is_same_v<T, varuint64_t>;
template <typename T>
concept sintable_t = std::is_same_v<T, sint32_t> || std::is_same_v<T, sint64_t>;

template <typename T>
constexpr auto get_field_varint_type(const T& t) {
  if constexpr (detail::optional<T>) {
    return get_field_varint_type(std::declval<typename T::value_type>());
  }
  else if constexpr (std::is_enum_v<T>) {
    return uint64_t{};
  }
  else if constexpr (varintable_t<T>) {
    return typename T::value_type{};
  }
  else if constexpr (sintable_t<T>) {
    return typename T::value_type{};
  }
  else {
    static_assert(!sizeof(T), "error field");
  }
}

template <typename T>
using field_varint_t = decltype(get_field_varint_type(std::declval<T>()));

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
    || std::same_as<T, sint32_t>
    || std::same_as<T, sint64_t>
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
concept I32 = std::same_as<T, int32_t>
    || std::same_as<T, uint32_t>
    || std::same_as<T, float>
;

constexpr static std::size_t MaxFieldNumber = 15;
enum class wire_type_t : uint8_t { varint, i64, len, sgroup, egroup, i32 };
template <typename T>
consteval wire_type_t get_wire_type() {
  if constexpr (detail::optional<T>) {
    return get_wire_type<typename std::remove_cvref_t<T>::value_type>();
  }
  else if constexpr (VARINT<T>) {
    return wire_type_t::varint;
  }
  else if constexpr (I64<T>) {
    return wire_type_t::i64;
  }
  else if constexpr (LEN<T>) {
    return wire_type_t::len;
  }
  else if constexpr (I32<T>) {
    return wire_type_t::i32;
  }
  else {
    static_assert(!sizeof(T), "SGROUP and EGROUP are deprecated");
  }
}
// clang-format on
template <typename T, std::size_t FieldIndex>
auto& get_field(T& t) {
  static_assert(!detail::optional<T>);
  constexpr auto Count = detail::member_count<T>();
  constexpr auto Index = FieldIndex;
  static_assert(Index >= 0 && Index <= Count);
  if constexpr (Count == 1) {
    auto& [a1] = t;
    return std::get<Index>(std::forward_as_tuple(a1));
  }
  else if constexpr (Count == 2) {
    auto& [a1, a2] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2));
  }
  else if constexpr (Count == 3) {
    auto& [a1, a2, a3] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3));
  }
  else if constexpr (Count == 4) {
    auto& [a1, a2, a3, a4] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4));
  }
  else if constexpr (Count == 5) {
    auto& [a1, a2, a3, a4, a5] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5));
  }
  else if constexpr (Count == 6) {
    auto& [a1, a2, a3, a4, a5, a6] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6));
  }
  else if constexpr (Count == 7) {
    auto& [a1, a2, a3, a4, a5, a6, a7] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7));
  }
  else if constexpr (Count == 8) {
    auto& [a1, a2, a3, a4, a5, a6, a7, a8] = t;
    return std::get<Index>(
        std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8));
  }
  else if constexpr (Count == 9) {
    auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9] = t;
    return std::get<Index>(
        std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9));
  }
  else if constexpr (Count == 10) {
    auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10] = t;
    return std::get<Index>(
        std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
  }
  else if constexpr (Count == 11) {
    auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11] = t;
    return std::get<Index>(
        std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11));
  }
  else if constexpr (Count == 12) {
    auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8,
                                                 a9, a10, a11, a12));
  }
  else if constexpr (Count == 13) {
    auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8,
                                                 a9, a10, a11, a12, a13));
  }
  else if constexpr (Count == 14) {
    auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8,
                                                 a9, a10, a11, a12, a13, a14));
  }
  else if constexpr (Count == 15) {
    auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15] =
        t;
    return std::get<Index>(std::forward_as_tuple(
        a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15));
  }
  else {
    static_assert(!sizeof(T), "wait for add hard code");
  }
}
template <typename T, std::size_t FieldIndex>
const auto& get_field(const T& t) {
  static_assert(!detail::optional<T>);
  constexpr auto Count = detail::member_count<T>();
  constexpr auto Index = FieldIndex;
  static_assert(Index >= 0 && Index <= Count);
  if constexpr (Count == 1) {
    auto&& [a1] = t;
    return std::get<Index>(std::forward_as_tuple(a1));
  }
  else if constexpr (Count == 2) {
    auto&& [a1, a2] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2));
  }
  else if constexpr (Count == 3) {
    auto&& [a1, a2, a3] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3));
  }
  else if constexpr (Count == 4) {
    auto&& [a1, a2, a3, a4] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4));
  }
  else if constexpr (Count == 5) {
    auto&& [a1, a2, a3, a4, a5] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5));
  }
  else if constexpr (Count == 6) {
    auto&& [a1, a2, a3, a4, a5, a6] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6));
  }
  else if constexpr (Count == 7) {
    auto&& [a1, a2, a3, a4, a5, a6, a7] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7));
  }
  else if constexpr (Count == 8) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8] = t;
    return std::get<Index>(
        std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8));
  }
  else if constexpr (Count == 9) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9] = t;
    return std::get<Index>(
        std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9));
  }
  else if constexpr (Count == 10) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10] = t;
    return std::get<Index>(
        std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
  }
  else if constexpr (Count == 11) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11] = t;
    return std::get<Index>(
        std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11));
  }
  else if constexpr (Count == 12) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8,
                                                 a9, a10, a11, a12));
  }
  else if constexpr (Count == 13) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8,
                                                 a9, a10, a11, a12, a13));
  }
  else if constexpr (Count == 14) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14] = t;
    return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8,
                                                 a9, a10, a11, a12, a13, a14));
  }
  else if constexpr (Count == 15) {
    auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15] =
        t;
    return std::get<Index>(std::forward_as_tuple(
        a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15));
  }
  else {
    static_assert(!sizeof(T), "wait for add hard code");
  }
}

template <std::size_t FieldNumber, typename T>
std::size_t STRUCT_PACK_INLINE calculate_one_size(const T& t);

template <typename T>
consteval auto get_field_number_to_index_map();

template <typename T, std::size_t Size, std::size_t... I>
std::size_t STRUCT_PACK_INLINE get_needed_size_impl(const T& t,
                                                    std::index_sequence<I...>) {
  constexpr auto Map = get_field_number_to_index_map<T>();
  constexpr auto i2n_map = Map.second;
  std::array<std::size_t, Size> size_array{
      calculate_one_size<i2n_map.at(I)>(get_field<T, I>(t))...};
  return std::accumulate(size_array.begin(), size_array.end(), 0);
}
template <typename T>
std::size_t STRUCT_PACK_INLINE get_needed_size(const T& t) {
  static_assert(std::is_class_v<T>);
  constexpr auto Count = detail::member_count<T>();
  return get_needed_size_impl<T, Count>(t, std::make_index_sequence<Count>());
}
template <typename U, typename T, unsigned Shift>
U STRUCT_PACK_INLINE encode_zigzag(T t) {
  return (static_cast<U>(t) << 1U) ^
         static_cast<U>(-static_cast<T>(static_cast<U>(t) >> Shift));
}
template <typename T>
auto STRUCT_PACK_INLINE encode_zigzag(T t) {
  if constexpr (std::is_same_v<T, int32_t>) {
    return encode_zigzag<uint32_t, int32_t, 31U>(t);
  }
  else if constexpr (std::is_same_v<T, int64_t>) {
    return encode_zigzag<uint64_t, int64_t, 63U>(t);
  }
  else {
    static_assert(!sizeof(T), "error zigzag type");
  }
}
template <typename T, typename U>
T STRUCT_PACK_INLINE decode_zigzag(U u) {
  return static_cast<T>((u >> 1U)) ^ static_cast<U>(-static_cast<T>(u & 1U));
}
template <typename T>
T STRUCT_PACK_INLINE decode_zigzag(T t) {
  if constexpr (std::is_same_v<T, int32_t>) {
    return decode_zigzag<int32_t, uint32_t>(t);
  }
  else if constexpr (std::is_same_v<T, int64_t>) {
    return decode_zigzag<int64_t, uint64_t>(t);
  }
  else {
    static_assert(!sizeof(T), "error type of zigzag");
  }
}
template <typename T>
constexpr std::size_t STRUCT_PACK_INLINE calculate_varint_size(T t) {
  if constexpr (std::unsigned_integral<T>) {
    std::size_t ret = 0;
    do {
      ret++;
      t >>= 7;
    } while (t != 0);
    return ret;
  }
  else if constexpr (varintable_t<T>) {
    using value_type = typename T::value_type;
    uint64_t v = value_type(t);
    return calculate_varint_size(v);
  }
  else if constexpr (sintable_t<T>) {
    using value_type = typename T::value_type;
    auto v = encode_zigzag(value_type(t));
    return calculate_varint_size(v);
  }
  else {
    static_assert(!sizeof(T), "error type");
  }
}
template <std::size_t FieldNumber, wire_type_t wire_type>
consteval std::size_t calculate_tag_size() {
  auto tag = (FieldNumber << 3U) | uint8_t(wire_type);
  return calculate_varint_size(tag);
}
template <std::size_t FieldNumber, typename T>
std::size_t STRUCT_PACK_INLINE calculate_one_size(const T& t) {
  constexpr auto wire_type = get_wire_type<T>();
  constexpr auto tag_size = calculate_tag_size<FieldNumber, wire_type>();
  if constexpr (detail::optional<T>) {
    if (t.has_value()) {
      return calculate_one_size<FieldNumber>(t.value());
    }
    else {
      return 0;
    }
  }
  else if constexpr (VARINT<T>) {
    if constexpr (std::is_enum_v<T>) {
      auto v = static_cast<uint64_t>(t);
      if (v == 0) {
        return 0;
      }
      return tag_size + calculate_varint_size(v);
    }
    else {
      if (t == T{}) {
        return 0;
      }
      return tag_size + calculate_varint_size(t);
    }
  }
  else if constexpr (LEN<T>) {
    if constexpr (std::same_as<T, std::string>) {
      if (t.empty()) {
        return 0;
      }
      return tag_size + calculate_varint_size(t.size()) + t.size();
    }
    else if constexpr (detail::container<T>) {
      if (t.empty()) {
        return 0;
      }
      using value_type = typename T::value_type;
      if constexpr (VARINT<value_type>) {
        std::size_t sz = 0;
        for (auto&& i : t) {
          sz += calculate_varint_size(i);
        }
        return tag_size + calculate_varint_size(t.size()) + sz;
      }
      else if constexpr (I32<value_type> || I64<value_type>) {
        auto sz = t.size();
        return tag_size + calculate_varint_size(sz) + sz * sizeof(value_type);
      }
      else {
        if constexpr (detail::map_container<T>) {
          using key_type = typename T::key_type;
          using mapped_type = typename T::mapped_type;
          static_assert(
              std::same_as<key_type, std::string> || std::integral<key_type>,
              "the key_type must be integral or string type");
          static_assert(!detail::map_container<mapped_type>,
                        "the mapped_type can be any type except another map.");
        }
        std::size_t total = 0;
        for (auto&& e : t) {
          auto size = get_needed_size(e);
          total += tag_size + calculate_varint_size(size) + size;
        }
        return total;
      }
    }
    else if constexpr (std::is_class_v<T>) {
      auto size = get_needed_size(t);
      return tag_size + calculate_varint_size(size) + size;
    }
    else {
      static_assert(!sizeof(T), "ERROR type");
      return 0;
    }
  }
  else if constexpr (I64<T>) {
    static_assert(sizeof(T) == 8);
    if (t == 0) {
      return 0;
    }
    return tag_size + sizeof(T);
  }
  else if constexpr (I32<T>) {
    static_assert(sizeof(T) == 4);
    if (t == 0) {
      return 0;
    }
    return tag_size + sizeof(T);
  }
  else {
    static_assert(!sizeof(T), "ERROR type");
    return 0;
  }
}
template <typename T>
constexpr std::size_t first_field_number = 1;

template <typename T>
struct field_number_array {
  using array_type = std::array<std::size_t, detail::member_count<T>()>;
};
template <typename T>
using field_number_array_t = typename field_number_array<T>::array_type;
template <typename T>
constexpr field_number_array_t<T> field_number_seq{};

template <typename... Args>
constexpr bool all_field_number_greater_than_zero(Args... args) {
  return (... && args);
}

template <typename... Args>
constexpr bool field_number_seq_not_init(Args... args) {
  return (... && (args == 0));
}

template <typename T>
constexpr bool has_duplicate_element() {
  constexpr auto seq = field_number_seq<T>;
  // TODO: a better way to check duplicate
  for (int i = 0; i < seq.size(); ++i) {
    for (int j = 0; j < i; ++j) {
      if (seq[i] == seq[j]) {
        return true;
      }
    }
  }
  return false;
}

template <typename T, std::size_t Size, std::size_t... I>
constexpr auto get_field_number_to_index_map_impl(std::index_sequence<I...>) {
  constexpr auto seq = field_number_seq<T>;
  if constexpr (field_number_seq_not_init(std::get<I>(seq)...)) {
    frozen::map<std::size_t, std::size_t, Size> n2i{
        {first_field_number<T> + I, I}...};
    frozen::map<std::size_t, std::size_t, Size> i2n{
        {I, first_field_number<T> + I}...};
    return std::make_pair(n2i, i2n);
  }
  else {
    static_assert(all_field_number_greater_than_zero(std::get<I>(seq)...),
                  "all field number must be specified");
    static_assert(!has_duplicate_element<T>());
    static_assert(first_field_number<T> == 1,
                  "field_number_seq and first_field_number cannot be specified "
                  "at the same time");
    auto field_number_set = frozen::make_set(seq);
    static_assert(field_number_set.size() == Size);
    frozen::map<std::size_t, std::size_t, Size> n2i{{std::get<I>(seq), I}...};
    frozen::map<std::size_t, std::size_t, Size> i2n{{I, std::get<I>(seq)}...};
    return std::make_pair(n2i, i2n);
  }
}
template <typename T>
consteval auto get_field_number_to_index_map() {
  constexpr auto Count = detail::member_count<T>();
  return get_field_number_to_index_map_impl<T, Count>(
      std::make_index_sequence<Count>());
}

template <typename T, std::size_t Size, std::size_t... I>
consteval auto get_sorted_field_number_array_impl(std::index_sequence<I...>) {
  constexpr auto Map = get_field_number_to_index_map<T>();
  constexpr auto i2n_map = Map.second;
  std::array<std::size_t, Size> array{i2n_map.at(I)...};
  std::sort(array.begin(), array.end());
  return array;
}

template <typename T>
consteval auto get_sorted_field_number_array() {
  constexpr auto Count = detail::member_count<T>();
  return get_sorted_field_number_array_impl<T, Count>(
      std::make_index_sequence<Count>());
}

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
    constexpr auto FieldArray = get_sorted_field_number_array<T>();
    constexpr auto Map = get_field_number_to_index_map<T>();
    constexpr auto n2i_map = Map.first;
    (serialize(get_field<T, n2i_map.at(std::get<I>(FieldArray))>(t),
               std::get<I>(FieldArray)),
     ...);
  }
  template <typename T>
  void serialize(const T& t, std::size_t field_number) {
    if constexpr (detail::optional<T>) {
      if (t.has_value()) {
        serialize(t.value(), field_number);
      }
      else {
        return;
      }
    }
    else {
      constexpr auto wire_type = get_wire_type<T>();
      if constexpr (VARINT<T>) {
        if constexpr (std::is_enum_v<T>) {
          auto v = static_cast<uint64_t>(t);
          if (v == 0) {
            return;
          }
          assert(pos_ < max_);
          // why '0'
          data_[pos_++] = '0';
          serialize_varint(v);
        }
        else {
          if (t == T{}) {
            return;
          }
          write_tag(field_number, wire_type);
          serialize_varint(t);
        }
      }
      else if constexpr (I64<T> || I32<T>) {
        if (t == 0) {
          return;
        }
        write_tag(field_number, wire_type);
        std::memcpy(data_ + pos_, &t, sizeof(T));
        pos_ += sizeof(T);
      }
      else if constexpr (LEN<T>) {
        if constexpr (std::same_as<T, std::string>) {
          if (t.empty()) {
            return;
          }
          write_tag(field_number, wire_type);
          serialize_varint(t.size());
          assert(pos_ + t.size() <= max_);
          std::memcpy(data_ + pos_, t.data(), t.size());
          pos_ += t.size();
        }
        else if constexpr (detail::map_container<T> || detail::container<T>) {
          if (t.empty()) {
            return;
          }
          using value_type = typename T::value_type;
          if constexpr (VARINT<value_type>) {
            write_tag(field_number, wire_type);
            auto sz_pos = pos_;
            // risk to data len > 1byte
            pos_++;
            std::size_t sz = 0;
            for (auto&& e : t) {
              sz += calculate_varint_size(e);
              serialize_varint(e);
            }
            pos_ = sz_pos;
            auto new_pos = pos_;
            serialize_varint(sz);
            pos_ = new_pos;
          }
          else if constexpr (I64<value_type> || I32<value_type>) {
            write_tag(field_number, wire_type);
            auto size = t.size() * sizeof(value_type);
            serialize_varint(size);
            std::memcpy(data_ + pos_, t.data(), size);
            pos_ += size;
          }
          else {
            for (auto&& e : t) {
              write_tag(field_number, wire_type);
              std::size_t sz = 0;
              if constexpr (I32<value_type> || I64<value_type>) {
                sz = calculate_one_size(e);
              }
              else {
                sz = get_needed_size(e);
              }
              serialize_varint(sz);
              serialize(e);
            }
          }
        }
        else {
          static_assert(std::is_class_v<T>);
          write_tag(field_number, wire_type);
          auto sz = get_needed_size(t);
          serialize_varint(sz);
          serialize(t);
        }
      }
      else {
        static_assert(!sizeof(T), "SGROUP and EGROUP are deprecated");
      }
    }
  }
  template <typename T>
  void serialize_varint(T t) {
    if constexpr (varintable_t<T>) {
      using value_type = std::make_unsigned_t<typename T::value_type>;
      serialize_varint(uint64_t(t));
    }
    else if constexpr (sintable_t<T>) {
      using value_type = typename T::value_type;
      auto v = encode_zigzag(value_type(t));
      serialize_varint(v);
    }
    else {
      uint64_t v = t;
      do {
        assert(pos_ < max_);
        data_[pos_++] = 0b1000'0000 | uint8_t(v);
        v >>= 7;
      } while (v != 0);
      assert(pos_ > 0);
      data_[pos_ - 1] = uint8_t(data_[pos_ - 1]) & 0b0111'1111;
    }
  }
  void write_tag(std::size_t field_number, wire_type_t wire_type) {
    auto tag = (field_number << 3) | uint8_t(wire_type);
    serialize_varint(tag);
  }

 private:
  Byte* data_;
  std::size_t pos_ = 0;
  std::size_t max_;
};

template <detail::struct_pack_byte Byte>
class unpacker {
 public:
  unpacker(const Byte* data, std::size_t size) : data_(data), size_(size) {}
  template <typename T>
  constexpr std::errc deserialize(T& t) {
    if (size_ == 0) [[unlikely]] {
      return std::errc{};
    }
    while (pos_ < size_) {
      auto ec = deserialize_one(t);
      if (ec != std::errc{}) {
        return ec;
      }
    }
    return std::errc{};
  }
  [[nodiscard]] std::size_t consume_len() const { return pos_; }

 private:
  template <typename T>
  constexpr std::errc deserialize_one(T& t) {
    constexpr auto Count = detail::member_count<T>();
    assert(pos_ < size_);
    auto tag = data_[pos_];
    uint8_t field_number = uint8_t(data_[pos_]) >> 3;
    if (field_number <= MaxFieldNumber) {
    }
    else {
      std::cout << field_number << std::endl;
      assert(field_number <= MaxFieldNumber);
    }
    auto wire_type =
        static_cast<wire_type_t>(uint8_t(data_[pos_]) & 0b0000'0111);
    pos_++;
    if (field_number == 1) {
      const auto FieldNumber = 1;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 2) {
      const auto FieldNumber = 2;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 3) {
      const auto FieldNumber = 3;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 4) {
      const auto FieldNumber = 4;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 5) {
      const auto FieldNumber = 5;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 6) {
      const auto FieldNumber = 6;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 7) {
      const auto FieldNumber = 7;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 8) {
      const auto FieldNumber = 8;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 9) {
      const auto FieldNumber = 9;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 10) {
      const auto FieldNumber = 10;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 11) {
      const auto FieldNumber = 11;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 12) {
      const auto FieldNumber = 12;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 13) {
      const auto FieldNumber = 13;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 14) {
      const auto FieldNumber = 14;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else if (field_number == 15) {
      const auto FieldNumber = 15;
      return deserialize_one<T, FieldNumber>(t, wire_type);
    }
    else {
      std::cout << "field number: " << field_number << std::endl;
      std::cout << "message field number: ";
      constexpr auto Map = get_field_number_to_index_map<T>();
      constexpr auto i2n_map = Map.second;
      for (int i = 0; i < Count; i++) {
        std::cout << i2n_map.at(i) << " ";
      }
      std::cout << std::endl;
      std::cout << "member count: " << Count << std::endl;
      assert(false && "not support now");
      return std::errc::function_not_supported;
    }
  }

  template <typename T, std::size_t FieldNumber>
  std::errc deserialize_one(T& t, wire_type_t wire_type) {
    constexpr auto Count = detail::member_count<T>();
    constexpr auto Map = get_field_number_to_index_map<T>();
    constexpr auto n2i_map = Map.first;
    constexpr auto i2n_map = Map.second;
    if constexpr (n2i_map.count(FieldNumber) == 0) {
      std::cout << "field number: " << FieldNumber << std::endl;
      std::cout << "message field number: ";
      for (int i = 0; i < Count; i++) {
        std::cout << i2n_map.at(i) << " ";
      }
      std::cout << std::endl;
      std::cout << "member count: " << Count << std::endl;
      assert(false && "not support now");
      return std::errc::invalid_argument;
    }
    else {
      constexpr auto I = n2i_map.at(FieldNumber);
      if constexpr (I < Count) {
        static_assert(I < Count);
        using T_Field = std::tuple_element_t<I, decltype(detail::get_types(
                                                    std::declval<T>()))>;
        constexpr auto field_wire_type = get_wire_type<T_Field>();
        if (field_wire_type != wire_type) {
          return std::errc::invalid_argument;
        }
        return deserialize_one<T, FieldNumber, field_wire_type>(t);
      }
      else {
        std::cout << "field number: " << FieldNumber << std::endl;
        std::cout << "message field number: ";
        for (int i = 0; i < Count; i++) {
          std::cout << i2n_map.at(i) << " ";
        }
        std::cout << std::endl;
        std::cout << "member count: " << Count << std::endl;
        assert(false && "not support now");
        return std::errc::invalid_argument;
      }
    }
  }

  template <typename T, std::size_t FieldNumber, wire_type_t WireType>
  std::errc deserialize_one(T& t) {
    static_assert(!std::is_const_v<T>);
    constexpr auto Map = get_field_number_to_index_map<T>();
    constexpr auto n2i_map = Map.first;
    static_assert(n2i_map.count(FieldNumber) == 1);
    constexpr auto FieldIndex = n2i_map.at(FieldNumber);
    auto&& f = get_field<T, FieldIndex>(t);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(f)>>);
    if constexpr (WireType == wire_type_t::varint) {
      using field_type = std::remove_reference_t<decltype(f)>;
      using value_type = field_varint_t<field_type>;
      value_type v = 0;
      auto ec = deserialize_varint(t, v);
      if constexpr (sintable_t<field_type>) {
        v = decode_zigzag(v);
      }
      if (ec == std::errc{}) {
        if constexpr (detail::optional<field_type>) {
          using optional_value_type = typename field_type::value_type;
          if constexpr (std::is_enum_v<optional_value_type>) {
            f = static_cast<optional_value_type>(v);
          }
          else {
            f = v;
          }
        }
        else if constexpr (std::is_enum_v<field_type>) {
          f = static_cast<field_type>(v);
        }
        else {
          f = v;
        }
      }
      return ec;
    }
    else if constexpr (WireType == wire_type_t::len) {
      return deserialize_len(t, f);
    }
    else if constexpr (WireType == wire_type_t::i32 ||
                       WireType == wire_type_t::i64) {
      return deserialize_fixedint(t, f);
    }
    else {
      return std::errc::invalid_argument;
    }
  }
  template <typename T, typename Field>
  std::errc deserialize_varint(T& t, Field& f) {
    if constexpr (detail::optional<Field>) {
      return deserialize_varint(t, f.value());
    }
    else {
      // Variable-width integers, or varints,
      // are at the core of the wire format.
      // They allow encoding unsigned 64-bit integers using anywhere
      // between one and ten bytes, with small values using fewer bytes.
      uint64_t n = 0;
      std::size_t i = 0;
      bool finished = false;
      while (pos_ < size_) {
        if ((uint8_t(data_[pos_]) >> 7) == 1) {
          n |= static_cast<uint64_t>(data_[pos_] & 0b0111'1111) << 7 * i;
          pos_++;
          i++;
        }
        else {
          finished = true;
          break;
        }
      }
      if (finished) {
        n |= static_cast<Field>(data_[pos_] & 0b0111'1111) << 7 * i;
        pos_++;
        f = n;
        return std::errc{};
      }
      else {
        if (pos_ >= size_) {
          return std::errc::no_buffer_space;
        }
        return std::errc::invalid_argument;
      }
    }
  }

  template <typename T, typename Field>
  std::errc deserialize_fixedint(T& t, Field& f) {
    if constexpr (detail::optional<Field>) {
      return deserialize_fixedint(t, f.value());
    }
    else {
      if (pos_ + sizeof(Field) > size_) {
        return std::errc::no_buffer_space;
      }
      assert(pos_ + sizeof(Field) <= size_);
      std::memcpy(&f, data_ + pos_, sizeof(Field));
      pos_ += sizeof(Field);
      return std::errc{};
    }
  }

  template <typename T, typename Field>
  std::errc deserialize_len(T& t, Field& f) {
    if constexpr (detail::optional<Field>) {
      using value_type = typename Field::value_type;
      value_type inner;
      auto ec = deserialize_len(t, inner);
      if (ec != std::errc{}) {
        return ec;
      }
      // TODO: copy?
      f = inner;
      return ec;
    }
    else if constexpr (std::same_as<Field, std::string>) {
      uint64_t sz = 0;
      auto ec = deserialize_varint(t, sz);
      if (ec != std::errc{}) {
        return ec;
      }
      f.resize(sz);
      std::memcpy(f.data(), data_ + pos_, sz);
      pos_ += sz;
      return std::errc{};
    }
    else if constexpr (detail::container<Field>) {
      uint64_t sz = 0;
      auto ec = deserialize_varint(t, sz);
      if (ec != std::errc{}) {
        return ec;
      }
      using value_type = typename Field::value_type;
      if constexpr (VARINT<value_type>) {
        auto max_pos = pos_ + sz;
        while (pos_ < max_pos) {
          uint64_t val = 0;
          ec = deserialize_varint(t, val);
          if (ec != std::errc{}) {
            return ec;
          }
          f.push_back(val);
        }
        return ec;
      }
      else if constexpr (I64<value_type> || I32<value_type>) {
        auto element_size = sz / sizeof(value_type);
        if (element_size * sizeof(value_type) != sz) {
          return std::errc::invalid_argument;
        }
        if (pos_ + sz > size_) {
          return std::errc::no_buffer_space;
        }
        f.resize(element_size);
        std::memcpy(&f[0], data_ + pos_, sz);
        pos_ += sz;
        return std::errc{};
      }
      else {
        if constexpr (detail::map_container<Field>) {
          using key_type = typename Field::key_type;
          using mapped_type = typename Field::mapped_type;
          // TODO: inplace?
          std::pair<key_type, mapped_type> entry{};
          unpacker o(data_ + pos_, sz);
          ec = o.template deserialize(entry);
          if (ec != std::errc{}) {
            return ec;
          }
          f[entry.first] = entry.second;
        }
        else {
          value_type val{};
          unpacker o(data_ + pos_, sz);
          ec = o.template deserialize(val);
          if (ec != std::errc{}) {
            return ec;
          }
          f.push_back(val);
        }
        pos_ += sz;
        return ec;
      }
    }
    else {
      uint64_t sz = 0;
      auto ec = deserialize_varint(t, sz);
      if (ec != std::errc{}) {
        return ec;
      }
      unpacker o(data_ + pos_, sz);
      ec = o.template deserialize(f);
      if (ec != std::errc{}) {
        return ec;
      }
      pos_ += sz;
      return ec;
    }
  }

 private:
  const Byte* data_;
  std::size_t size_;
  std::size_t pos_ = 0;
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
template <detail::struct_pack_buffer Buffer, typename... Args>
void STRUCT_PACK_INLINE serialize_to(Buffer& buffer, const Args&... args) {
  static_assert(sizeof...(args) == 1);
  auto data_offset = buffer.size();
  auto need_size = get_needed_size(args...);
  auto total = data_offset + need_size;
  buffer.resize(total);
  packer o(buffer.data() + data_offset, need_size);
  o.serialize(args...);
}
template <typename T, detail::struct_pack_byte Byte>
auto STRUCT_PACK_INLINE deserialize(const Byte* data, std::size_t size,
                                    std::size_t& consume_len) {
  expected<T, std::errc> ret;
  unpacker o(data, size);
  auto ec = o.deserialize(ret.value());
  if (ec != std::errc{}) {
    ret = struct_pack::unexpected<std::errc>{ec};
  }
  else {
    consume_len = o.consume_len();
  }
  return ret;
}
}  // namespace pb
}  // namespace struct_pack