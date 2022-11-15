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

#include "struct_pack/struct_pack.hpp"
#include "struct_pack/struct_pack/reflection.h"
#include "struct_pack/struct_pack/struct_pack_impl.hpp"
namespace struct_pack {
namespace pb {
template <typename T>
class varint {
  using value_type = T;

 public:
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
concept varintable =
    requires { std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>; };

using varint32_t = varint<int32_t>;
using varuint32_t = varint<uint32_t>;
using varint64_t = varint<int64_t>;
using varuint64_t = varint<uint64_t>;

template <typename T>
concept Enum = requires { std::is_enum_v<T>; };

template <Enum T>
struct field_varint {
  using value_type = uint64_t;
};

template <varintable T>
struct field_varint<varint<T>> {
  using value_type = T;
};

template <varintable T>
struct field_varint<std::optional<varint<T>>> {
  using value_type = T;
};

template <typename T>
using field_varint_t = typename field_varint<T>::value_type;

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
    || std::same_as<T, float>
;

constexpr static std::size_t MaxFieldNumber = 15;

// clang-format on
std::size_t STRUCT_PACK_INLINE calculate_needed_size() { return 0; }
template <typename T, typename... Args>
std::size_t STRUCT_PACK_INLINE calculate_needed_size(const T& t,
                                                     const Args&... args);
template <typename T>
std::size_t STRUCT_PACK_INLINE calculate_one_size(const T& t);
template <typename T>
std::size_t STRUCT_PACK_INLINE get_needed_size(const T& t) {
  static_assert(std::is_class_v<T>);
  std::size_t ret = 0;
  detail::visit_members(t, [&ret](auto&&... args) {
    ret += calculate_needed_size(args...);
  });
  return ret;
}
std::size_t STRUCT_PACK_INLINE calculate_varint_size(uint64_t t) {
  std::size_t ret = 0;
  do {
    ret++;
    t >>= 7;
  } while (t != 0);
  return ret;
}
template <typename T>
std::size_t STRUCT_PACK_INLINE calculate_one_size(const T& t) {
  if constexpr (detail::optional<T>) {
    if (t.has_value()) {
      return calculate_one_size(t.value());
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
      return 1 + calculate_varint_size(v);
    }
    else {
      if (t == 0) {
        return 0;
      }
      return 1 + calculate_varint_size(t);
    }
  }
  else if constexpr (LEN<T>) {
    if constexpr (std::same_as<T, std::string>) {
      if (t.empty()) {
        return 0;
      }
      return 1 + calculate_varint_size(t.size()) + t.size();
    }
    else if constexpr (detail::map_container<T> || detail::container<T>) {
      if (t.empty()) {
        return 0;
      }
      using value_type = typename T::value_type;
      if constexpr (VARINT<value_type>) {
        std::size_t sz = 0;
        for (auto&& i : t) {
          sz += calculate_varint_size(i);
        }
        return 1 + calculate_varint_size(t.size()) + sz;
      }
      else {
        std::size_t sz = 0;
        for (auto&& i : t) {
          sz += calculate_one_size(i);
        }
        return sz;
      }
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
    if (t == 0) {
      return 0;
    }
    return 1 + 8;
  }
  else if constexpr (I32<T>) {
    if (t == 0) {
      return 0;
    }
    return 1 + 4;
  }
  else {
    static_assert(!sizeof(T), "ERROR type");
    return 0;
  }
}
template <typename T, typename... Args>
std::size_t STRUCT_PACK_INLINE calculate_needed_size(const T& t,
                                                     const Args&... args) {
  auto size = calculate_one_size(t);
  return size + calculate_needed_size(args...);
}
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
    static_assert(FieldNumber <= MaxFieldNumber);
    static_assert(!detail::optional<T>);
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
    else if constexpr (Count == 4) {
      const auto& [a1, a2, a3, a4] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4));
    }
    else if constexpr (Count == 5) {
      const auto& [a1, a2, a3, a4, a5] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5));
    }
    else if constexpr (Count == 6) {
      const auto& [a1, a2, a3, a4, a5, a6] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6));
    }
    else if constexpr (Count == 7) {
      const auto& [a1, a2, a3, a4, a5, a6, a7] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7));
    }
    else if constexpr (Count == 8) {
      const auto& [a1, a2, a3, a4, a5, a6, a7, a8] = t;
      return std::get<Index>(
          std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8));
    }
    else if constexpr (Count == 9) {
      const auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9] = t;
      return std::get<Index>(
          std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9));
    }
    else if constexpr (Count == 10) {
      const auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10] = t;
      return std::get<Index>(
          std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
    }
    else if constexpr (Count == 11) {
      const auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11] = t;
      return std::get<Index>(
          std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11));
    }
    else if constexpr (Count == 12) {
      const auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7,
                                                   a8, a9, a10, a11, a12));
    }
    else if constexpr (Count == 13) {
      const auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7,
                                                   a8, a9, a10, a11, a12, a13));
    }
    else if constexpr (Count == 14) {
      const auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13,
                   a14] = t;
      return std::get<Index>(std::forward_as_tuple(
          a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14));
    }
    else if constexpr (Count == 15) {
      const auto& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14,
                   a15] = t;
      return std::get<Index>(std::forward_as_tuple(
          a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15));
    }
    else {
      static_assert(!sizeof(T), "wait for add hard code");
    }
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
          data_[pos_++] = '0';
          serialize_varint(v);
        }
        else {
          if (t == 0) {
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
          else {
            for (auto&& e : t) {
              write_tag(field_number, wire_type);
              auto sz = get_needed_size(e);
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
  void serialize_varint(uint64_t t) {
    do {
      assert(pos_ < max_);
      data_[pos_++] = 0b1000'0000 | uint8_t(t);
      t >>= 7;
    } while (t != 0);
    assert(pos_ > 0);
    data_[pos_ - 1] = uint8_t(data_[pos_ - 1]) & 0b0111'1111;
  }
  void write_tag(std::size_t field_number, wire_type_t wire_type) {
    auto tag = (field_number << 3) | uint8_t(wire_type);
    assert(pos_ < max_);
    data_[pos_++] = tag;
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
      std::cout << "first field number: " << first_field_number<T> << std::endl;
      std::cout << "member count: " << Count << std::endl;
      assert(false && "not support now");
      return std::errc::function_not_supported;
    }
  }

  template <typename T, std::size_t FieldNumber>
  std::errc deserialize_one(T& t, wire_type_t wire_type) {
    constexpr auto Count = detail::member_count<T>();
    if constexpr (FieldNumber < first_field_number<T> ||
                  Count < FieldNumber - first_field_number<T>) {
      std::cout << "field number: " << FieldNumber << std::endl;
      std::cout << "first field number: " << first_field_number<T> << std::endl;
      std::cout << "member count: " << Count << std::endl;
      assert(false && "not support now");
      return std::errc::invalid_argument;
    }
    else {
      constexpr auto I = FieldNumber - first_field_number<T>;
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
        std::cout << "first field number: "
                  << first_field_number<T> << std::endl;
        std::cout << "member count: " << Count << std::endl;
        assert(false && "not support now");
        return std::errc::invalid_argument;
      }
    }
  }
  template <typename T, std::size_t FieldNumber, wire_type_t WireType>
  std::errc deserialize_one(T& t) {
    static_assert(!std::is_const_v<T>);
    auto&& f = get_field<T, FieldNumber>(t);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(f)>>);
    if constexpr (WireType == wire_type_t::varint) {
      using field_type = std::remove_reference_t<decltype(f)>;
      using value_type = field_varint_t<field_type>;
      value_type v = 0;
      auto ec = deserialize_varint(t, v);
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
      Field n = 0;
      std::size_t i = 0;
      bool finished = false;
      while (pos_ < size_) {
        if ((uint8_t(data_[pos_]) >> 7) == 1) {
          n |= static_cast<Field>(data_[pos_] & 0b0111'1111) << 7 * i;
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
    else if constexpr (detail::map_container<Field> ||
                       detail::container<Field>) {
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
      else {
        value_type val{};
        unpacker o(data_ + pos_, sz);
        ec = o.template deserialize(val);
        if (ec != std::errc{}) {
          return ec;
        }
        f.push_back(val);
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

  template <typename T, std::size_t FieldNumber>
  auto&& get_field(T& t) {
    static_assert(FieldNumber <= MaxFieldNumber);
    static_assert(!detail::optional<T>);
    constexpr auto Count = detail::member_count<T>();
    constexpr std::size_t Index = FieldNumber - first_field_number<T>;
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
      return std::get<Index>(
          std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a12));
    }
    else if constexpr (Count == 13) {
      auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7,
                                                   a8, a9, a10, a12, a13));
    }
    else if constexpr (Count == 14) {
      auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14] = t;
      return std::get<Index>(std::forward_as_tuple(a1, a2, a3, a4, a5, a6, a7,
                                                   a8, a9, a10, a12, a13, a14));
    }
    else if constexpr (Count == 15) {
      auto&& [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14,
              a15] = t;
      return std::get<Index>(std::forward_as_tuple(
          a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a12, a13, a14, a15));
    }
    else {
      static_assert(!sizeof(T), "wait for add hard code");
    }
  }

  template <typename T, std::size_t FieldNumber>
  consteval decltype(auto) get_field_wire_type() {
    constexpr auto I = FieldNumber - first_field_number<T>;
    using T_Field =
        std::tuple_element_t<I, decltype(detail::get_types(std::declval<T>()))>;
    return get_wire_type<T_Field>();
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
template <typename T, detail::deserialize_view View>
[[nodiscard]] STRUCT_PACK_INLINE std::errc deserialize_to_with_offset(
    T& t, const View& v, size_t& offset) {
  unpacker in(v.data() + offset, v.size() - offset);
  auto ret = in.deserialize(t);
  offset += in.consume_len();
  return ret;
}
}  // namespace pb
}  // namespace struct_pack