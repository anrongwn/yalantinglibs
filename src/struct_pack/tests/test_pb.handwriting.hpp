#pragma once
#include <valarray>
#include "struct_pack/struct_pack/pb.hpp"
using namespace struct_pack::pb;
struct test1 {
  varint32_t a;
  auto operator<=>(const test1 &) const = default;
};
struct test2 {
  std::string b;
};
template <>
constexpr std::size_t first_field_number<test2> = 2;
struct test3 {
  std::optional<test1> c;
};
template <>
constexpr std::size_t first_field_number<test3> = 3;
struct test4 {
  std::string d;
  std::vector<varint32_t> e;
};
template <>
constexpr std::size_t first_field_number<test4> = 4;


struct my_test_double {
  double a;
  double b;
  double c;
  bool operator==(const my_test_double &rhs) const {
    std::valarray<double> lh({a, b, c});
    std::valarray<double> rh({rhs.a, rhs.b, rhs.c});
    return (std::abs(lh - rh) < 0.05f).min();
  };
};

struct my_test_float {
  float a;
  float b;
  float c;
  float d;
  float e;
  bool operator==(const my_test_float &rhs) const {
    std::valarray<float> lh({a, b, c, d, e});
    std::valarray<float> rh({rhs.a, rhs.b, rhs.c, rhs.d, rhs.e});
    return (std::abs(lh - rh) < 0.05f).min();
  };
};


struct my_test_int32 {
  std::optional<varint32_t> a;
};

struct my_test_int64 {
  varint64_t a;
  bool operator==(const my_test_int64 &) const = default;
};

struct my_test_uint32 {
  varuint64_t a;
  bool operator==(const my_test_uint32 &) const = default;
};

struct my_test_uint64 {
  varuint64_t a;
  bool operator==(const my_test_uint64 &) const = default;
};

struct my_test_enum {
  enum class Color { Red, Green, Blue, Enum127 = 127, Enum128 = 128 };
  Color color;
};
template <>
constexpr std::size_t first_field_number<my_test_enum> = 6;

struct my_test_repeated_message {
  std::vector<my_test_float> fs;
};

struct my_test_sint32 {
  sint32_t a;
  auto operator<=>(const my_test_sint32 &) const = default;
};

struct my_test_sint64 {
  sint64_t b;
  auto operator<=>(const my_test_sint64 &) const = default;
};
template <>
constexpr std::size_t first_field_number<my_test_sint64> = 2;

struct my_test_map {
  std::unordered_map<std::string, varint32_t> e;
};
template <>
constexpr std::size_t first_field_number<my_test_map> = 3;

template <typename T>
struct my_test_fixed {
  using value_type = T;
  T a;
  std::vector<T> b;
  bool operator==(const my_test_fixed &) const = default;
};

struct my_test_fixed32 {
  uint32_t a;
  std::vector<uint32_t> b;
  bool operator==(const my_test_fixed32 &) const = default;
};

struct my_test_field_number_random {
  varint32_t a;
  sint64_t b;
  std::string c;
  double d;
  float e;
  std::vector<uint32_t> f;
  bool operator==(const my_test_field_number_random &) const = default;
};
template <>
constexpr field_number_array_t<my_test_field_number_random>
    field_number_seq<my_test_field_number_random>{6, 3, 4, 5, 1, 128};


struct my_test_all {
  double a;
  float b;
  varint32_t c;
  varint64_t d;
  varuint32_t e;
  varuint64_t f;
  sint32_t g;
  sint64_t h;
  uint32_t i;
  uint64_t j;
  int32_t k;
  int64_t l;
  bool m;
  std::string n;
  std::string o;

  bool operator==(const my_test_all &rhs) const {
    return (std::abs(a - rhs.a) < 0.0005) && (std::abs(b - rhs.b) < 0.005) &&
           c == rhs.c && d == rhs.d && e == rhs.e && f == rhs.f && g == rhs.g &&
           h == rhs.h && i == rhs.i && j == rhs.j && k == rhs.k && l == rhs.l &&
           m == rhs.m && n == rhs.n && o == rhs.o;
  }
};


