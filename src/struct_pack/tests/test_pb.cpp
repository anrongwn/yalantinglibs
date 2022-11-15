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
#include <valarray>

#include "doctest.h"
#include "hex_printer.hpp"
#include "struct_pack/struct_pack/pb.hpp"
#include "test_pb.pb.h"
using namespace doctest;
using namespace struct_pack::pb;
struct test1 {
  varint32_t a;
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
TEST_SUITE_BEGIN("test pb");
TEST_CASE("testing test1") {
  test1 t{};
  SUBCASE("empty") {
    auto size = get_needed_size(t);
    REQUIRE(size == 0);
    auto b = serialize<std::string>(t);
    CHECK(b.empty());
    std::size_t len = 0;
    auto d_t_ret = deserialize<test1>(b.data(), b.size(), len);
    CHECK(len == 0);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
    CHECK(t.a == d_t.a);
  }
  SUBCASE("has value") {
    std::string buf{0x08, (char)0x96, 0x01};
    Test1 pb_t;
    pb_t.set_a(150);
    auto pb_buf = pb_t.SerializeAsString();
    REQUIRE(pb_buf == buf);

    t.a = 150;
    auto size = get_needed_size(t);
    REQUIRE(size == 3);
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
    std::size_t len = 0;
    auto d_t_ret = deserialize<test1>(b.data(), b.size(), len);
    CHECK(len == 3);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
    CHECK(t.a == d_t.a);
  }
}
TEST_CASE("testing test2") {
  test2 t{};
  SUBCASE("empty") {
    auto size = get_needed_size(t);
    REQUIRE(size == 0);
    auto b = serialize<std::string>(t);
    CHECK(b.empty());
    std::size_t len = 0;
    auto d_t_ret = deserialize<test1>(b.data(), b.size(), len);
    CHECK(len == 0);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
  }
  SUBCASE("has value") {
    std::string buf{0x12, 0x07};
    std::string s = "testing";
    buf += s;

    Test2 pb_t;
    pb_t.set_b(s);
    auto pb_buf = pb_t.SerializeAsString();
    REQUIRE(pb_buf == buf);

    t.b = s;
    auto size = get_needed_size(t);
    REQUIRE(size == 9);
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
    std::size_t len = 0;
    auto d_t_ret = deserialize<test2>(b.data(), b.size(), len);
    CHECK(len == 9);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
    CHECK(t.b == d_t.b);
  }
}
TEST_CASE("testing test3") {
  test3 t{};
  SUBCASE("empty") {
    auto size = get_needed_size(t);
    REQUIRE(size == 0);
    auto b = serialize<std::string>(t);
    CHECK(b.empty());
    std::size_t len = 0;
    auto d_t_ret = deserialize<test1>(b.data(), b.size(), len);
    CHECK(len == 0);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
  }
  SUBCASE("has value") {
    std::string buf{0x1a, 0x03, 0x08, (char)0x96, 0x01};

    Test3 pb_t;
    auto pb_c = new Test1;
    pb_c->set_a(150);
    pb_t.set_allocated_c(pb_c);
    auto pb_buf = pb_t.SerializeAsString();
    REQUIRE(pb_buf == buf);

    t.c = test1{.a = 150};
    auto size = get_needed_size(t);
    REQUIRE(size == 5);
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
    std::size_t len = 0;
    auto d_t_ret = deserialize<test3>(b.data(), b.size(), len);
    CHECK(len == 5);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
    REQUIRE(d_t.c.has_value());
    CHECK(t.c->a == d_t.c->a);
  }
}
TEST_CASE("testing test4") {
  test4 t{};
  SUBCASE("empty") {
    auto size = get_needed_size(t);
    REQUIRE(size == 0);
    auto b = serialize<std::string>(t);
    CHECK(b.empty());
    std::size_t len = 0;
    auto d_t_ret = deserialize<test1>(b.data(), b.size(), len);
    CHECK(len == 0);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
  }
  SUBCASE("string empty") {
    Test4 pb_t;
    pb_t.add_e(1);
    auto pb_buf = pb_t.SerializeAsString();
    REQUIRE(pb_buf.size() == 3);

    t.e = {1};
    auto size = get_needed_size(t);
    REQUIRE(size == 3);
    std::string buf{0x2a, 0x01, 0x01};
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
    std::size_t len = 0;
    auto d_t_ret = deserialize<test4>(b.data(), b.size(), len);
    CHECK(len == 3);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
    REQUIRE(!d_t.e.empty());
    CHECK(t.e.size() == d_t.e.size());
    CHECK(t.e[0] == d_t.e[0]);
  }
  SUBCASE("repeated empty") {
    // [4] 22 2 68 69
    std::string buf{0x22, 0x02, 0x68, 0x69};
    std::string s = "hi";

    Test4 pb_t;
    pb_t.set_d(s);
    auto pb_buf = pb_t.SerializeAsString();
    REQUIRE(pb_buf == buf);

    t.d = s;
    auto size = get_needed_size(t);
    REQUIRE(size == 4);
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
    std::size_t len = 0;
    auto d_t_ret = deserialize<test4>(b.data(), b.size(), len);
    CHECK(len == 4);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
    CHECK(t.d == d_t.d);
  }
  SUBCASE("has value") {
    Test4 pb_t;
    pb_t.set_d("hello");
    pb_t.add_e(1);
    pb_t.add_e(2);
    pb_t.add_e(3);
    auto pb_buf = pb_t.SerializeAsString();
    REQUIRE(pb_buf.size() == 12);

    t.d = "hello";
    t.e = {1, 2, 3};
    auto size = get_needed_size(t);
    REQUIRE(size == 12);
    std::string buf{0x22, 0x05, 0x68, 0x65, 0x6c, 0x6c,
                    0x6f, 0x2a, 0x03, 0x01, 0x02, 0x03};
    // why document write
    // 220568656c6c6f280128022803
    // while my pb c++ code generated
    // 34 5 'h' 'e' 'l' 'l' 'o' 42 3 1 2 3
    // 22 5 68 65 6c 6c 6f 2a 3 1 2 3
    auto b = serialize<std::string>(t);
    REQUIRE(buf == b);
    std::size_t len = 0;
    auto d_t_ret = deserialize<test4>(b.data(), b.size(), len);
    CHECK(len == 12);
    REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
    auto d_t = d_t_ret.value();
    CHECK(t.d == d_t.d);
    CHECK(t.e == d_t.e);
  }
}
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
TEST_CASE("testing float") {
  // [20] 15 00 00 80 3f 1d 00 00 80 bf 25 b6 f3 9d 3f 2d 00 04 f1 47
  std::string buf{0x15, 0x00,       0x00,       (char)0x80, 0x3f,
                  0x1d, 0x00,       0x00,       (char)0x80, (char)0xbf,
                  0x25, (char)0xb6, (char)0xf3, (char)0x9d, 0x3f,
                  0x2d, 0x00,       0x04,       (char)0xf1, 0x47};
  MyTestFloat pb_t;
  pb_t.set_a(0);
  pb_t.set_b(1);
  pb_t.set_c(-1);
  pb_t.set_d(1.234);
  pb_t.set_e(1.234e5);
  auto pb_buf = pb_t.SerializeAsString();
  REQUIRE(pb_buf == buf);

  my_test_float t{.a = 0, .b = 1, .c = -1, .d = 1.234, .e = 1.234e5};
  auto size = get_needed_size(t);
  REQUIRE(size == 20);
  auto b = serialize<std::string>(t);
  CHECK(buf == b);
  std::size_t len = 0;
  auto d_t_ret = deserialize<my_test_float>(b.data(), b.size(), len);
  CHECK(len == 20);
  REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
  auto d_t = d_t_ret.value();
  CHECK(t.a == d_t.a);
  CHECK(t.b == d_t.b);
  CHECK(t.c == d_t.c);
  CHECK(t.d == d_t.d);
  CHECK(t.e == d_t.e);
}

struct my_test_int32 {
  std::optional<varint32_t> a;
};

TEST_CASE("testing int32") {
  std::string buf{0x08, (char)0x80, 0x01};
  MyTestInt32 pb_t;
  pb_t.set_a(128);
  auto pb_buf = pb_t.SerializeAsString();
  REQUIRE(pb_buf == buf);

  my_test_int32 t;
  t.a = 128;

  auto size = get_needed_size(t);
  REQUIRE(size == pb_buf.size());

  auto b = serialize<std::string>(t);
  CHECK(b == buf);

  std::size_t len = 0;
  auto d_t_ret = deserialize<my_test_int32>(b.data(), b.size(), len);
  REQUIRE(len == b.size());
  REQUIRE_MESSAGE(d_t_ret, struct_pack::error_message(d_t_ret.error()));
  auto d_t = d_t_ret.value();
  CHECK(d_t.a == t.a);
}

struct my_test_enum {
  enum class Color { Red, Green, Blue, Enum127 = 127, Enum128 = 128 };
  Color color;
};
template <>
constexpr std::size_t first_field_number<my_test_enum> = 6;
TEST_CASE("testing enum") {
  SUBCASE("Red") {
    MyTestEnum pb_t;
    pb_t.set_color(MyTestEnum_Color::MyTestEnum_Color_Red);
    auto pb_buf = pb_t.SerializeAsString();
    MyTestEnum pb_d_t;
    auto ok = pb_d_t.ParseFromArray(pb_buf.data(), pb_buf.size());
    REQUIRE(ok);
    CHECK(pb_d_t.IsInitialized());
    CHECK(pb_d_t.color() == pb_t.color());
    auto c = pb_d_t.color();

    my_test_enum t{};
    t.color = my_test_enum::Color::Red;
    auto size = get_needed_size(t);
    REQUIRE(size == pb_buf.size());

    auto b = serialize<std::string>(t);
    CHECK(b == pb_buf);

    std::size_t len = 0;
    auto d_t_ret = deserialize<my_test_enum>(b.data(), b.size(), len);
    REQUIRE(len == b.size());
    REQUIRE(d_t_ret);
    auto d_t = d_t_ret.value();
    CHECK(d_t.color == my_test_enum::Color::Red);
    CHECK(t.color == d_t.color);
  }
  SUBCASE("Green") {
    MyTestEnum pb_t;
    pb_t.set_color(MyTestEnum_Color::MyTestEnum_Color_Green);
    auto pb_buf = pb_t.SerializeAsString();

    my_test_enum t{};
    t.color = my_test_enum::Color::Green;
    auto size = get_needed_size(t);
    REQUIRE(size == pb_buf.size());

    auto b = serialize<std::string>(t);
    CHECK(b == pb_buf);

    std::size_t len = 0;
    auto d_t_ret = deserialize<my_test_enum>(b.data(), b.size(), len);
    REQUIRE(len == b.size());
    REQUIRE(d_t_ret);
    auto d_t = d_t_ret.value();
    CHECK(d_t.color == my_test_enum::Color::Green);
    CHECK(t.color == d_t.color);
  }
  SUBCASE("Blue") {
    MyTestEnum pb_t;
    pb_t.set_color(MyTestEnum_Color::MyTestEnum_Color_Blue);
    auto pb_buf = pb_t.SerializeAsString();

    my_test_enum t{};
    t.color = my_test_enum::Color::Blue;
    auto size = get_needed_size(t);
    REQUIRE(size == pb_buf.size());

    auto b = serialize<std::string>(t);
    CHECK(b == pb_buf);

    std::size_t len = 0;
    auto d_t_ret = deserialize<my_test_enum>(b.data(), b.size(), len);
    REQUIRE(len == b.size());
    REQUIRE(d_t_ret);
    auto d_t = d_t_ret.value();
    CHECK(d_t.color == my_test_enum::Color::Blue);
    CHECK(t.color == d_t.color);
  }
  SUBCASE("Enum127") {
    MyTestEnum pb_t;
    pb_t.set_color(MyTestEnum_Color::MyTestEnum_Color_Enum127);
    auto pb_buf = pb_t.SerializeAsString();

    my_test_enum t{};
    t.color = my_test_enum::Color::Enum127;
    auto size = get_needed_size(t);
    REQUIRE(size == pb_buf.size());

    auto b = serialize<std::string>(t);
    CHECK(b == pb_buf);

    std::size_t len = 0;
    auto d_t_ret = deserialize<my_test_enum>(b.data(), b.size(), len);
    REQUIRE(len == b.size());
    REQUIRE(d_t_ret);
    auto d_t = d_t_ret.value();
    CHECK(d_t.color == my_test_enum::Color::Enum127);
    CHECK(t.color == d_t.color);
  }
  SUBCASE("Enum128") {
    MyTestEnum pb_t;
    pb_t.set_color(MyTestEnum_Color::MyTestEnum_Color_Enum128);
    auto pb_buf = pb_t.SerializeAsString();

    my_test_enum t{};
    t.color = my_test_enum::Color::Enum128;
    auto size = get_needed_size(t);
    REQUIRE(size == pb_buf.size());

    auto b = serialize<std::string>(t);
    CHECK(b == pb_buf);

    std::size_t len = 0;
    auto d_t_ret = deserialize<my_test_enum>(b.data(), b.size(), len);
    REQUIRE(len == b.size());
    REQUIRE(d_t_ret);
    auto d_t = d_t_ret.value();
    CHECK(d_t.color == my_test_enum::Color::Enum128);
    CHECK(t.color == d_t.color);
  }
}

struct my_test_repeated_message {
  std::vector<my_test_float> fs;
};
TEST_CASE("testing nested repeated message") {
  SUBCASE("one") {
    MyTestRepeatedMessage pb_t;
    {
      auto f1 = pb_t.add_fs();
      f1->set_a(1);
      f1->set_b(2);
      f1->set_c(3);
    }
    auto pb_b = pb_t.SerializeAsString();

    MyTestRepeatedMessage pb_d_t;
    auto ok = pb_d_t.ParseFromArray(pb_b.data(), pb_b.size());
    REQUIRE(ok);

    my_test_repeated_message t{};
    t.fs = {{1, 2, 3}};
    auto b = serialize<std::string>(t);
    REQUIRE(b == pb_b);
    std::size_t len = 0;
    auto d_t_ret =
        deserialize<my_test_repeated_message>(b.data(), b.size(), len);
    REQUIRE(len == b.size());
    REQUIRE(d_t_ret);
    auto d_t = d_t_ret.value();
    CHECK(t.fs == d_t.fs);
  }
  SUBCASE("two") {
    MyTestRepeatedMessage pb_t;
    {
      auto f1 = pb_t.add_fs();
      f1->set_a(1);
      f1->set_b(2);
      f1->set_c(3);
    }
    {
      auto f1 = pb_t.add_fs();
      f1->set_a(4);
      f1->set_b(5);
      f1->set_c(6);
    }
    auto pb_b = pb_t.SerializeAsString();

    MyTestRepeatedMessage pb_d_t;
    auto ok = pb_d_t.ParseFromArray(pb_b.data(), pb_b.size());
    REQUIRE(ok);

    my_test_repeated_message t{};
    t.fs = {{1, 2, 3}, {4, 5, 6}};
    auto b = serialize<std::string>(t);
    REQUIRE(b == pb_b);
    std::size_t len = 0;
    auto d_t_ret =
        deserialize<my_test_repeated_message>(b.data(), b.size(), len);
    REQUIRE(len == b.size());
    REQUIRE(d_t_ret);
    auto d_t = d_t_ret.value();
    CHECK(t.fs == d_t.fs);
  }
}

TEST_SUITE_END;