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
#include "doctest.h"
#include "struct_pack/struct_pack/pb.hpp"
using namespace doctest;
using namespace struct_pack::pb;
struct test1 {
  std::optional<varint32_t> a;
};
struct test2 {
  std::optional<std::string> b;
};
template <>
constexpr std::size_t first_field_number<test2> = 2;
struct test3 {
  std::optional<test1> c;
};
template <>
constexpr std::size_t first_field_number<test3> = 3;
struct test4 {
  std::optional<std::string> d;
  std::vector<varint32_t> e;
};
template <>
constexpr std::size_t first_field_number<test4> = 4;
TEST_SUITE_BEGIN("test pb");
TEST_CASE("testing test1") {
  test1 t;
  SUBCASE("empty") {
    auto size = get_needed_size(t);
    REQUIRE(size == 0);
    auto b = serialize<std::string>(t);
    CHECK(b.empty());
  }
  SUBCASE("has value") {
    t.a = 150;
    auto size = get_needed_size(t);
    REQUIRE(size == 3);
    std::string buf{0x08, (char)0x96, 0x01};
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
  }
}
TEST_CASE("testing test2") {
  test2 t;
  SUBCASE("empty") {
    auto size = get_needed_size(t);
    REQUIRE(size == 0);
    auto b = serialize<std::string>(t);
    CHECK(b.empty());
  }
  SUBCASE("has value") {
    t.b = "testing";
    auto size = get_needed_size(t);
    REQUIRE(size == 9);
    std::string buf{0x12, 0x07};
    buf += t.b.value();
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
  }
}
TEST_CASE("testing test3") {
  test3 t;
  SUBCASE("empty") {
    auto size = get_needed_size(t);
    REQUIRE(size == 0);
    auto b = serialize<std::string>(t);
    CHECK(b.empty());
  }
  SUBCASE("has value") {
    t.c = test1{.a = 150};
    auto size = get_needed_size(t);
    REQUIRE(size == 5);
    std::string buf{0x1a, 0x03, 0x08, (char)0x96, 0x01};
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
  }
}
TEST_CASE("testing test4") {
  test4 t;
  SUBCASE("empty") {
    auto size = get_needed_size(t);
    REQUIRE(size == 0);
    auto b = serialize<std::string>(t);
    CHECK(b.empty());
  }
  SUBCASE("string empty") {
    t.e = {1};
    auto size = get_needed_size(t);
    REQUIRE(size == 3);
    std::string buf{0x2a, 0x01, 0x01};
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
  }
  SUBCASE("repeated empty") {
    t.d = "hi";
    auto size = get_needed_size(t);
    REQUIRE(size == 4);
    // [4] 22 2 68 69
    std::string buf{0x22, 0x02, 0x68, 0x69};
    auto b = serialize<std::string>(t);
    CHECK(buf == b);
  }
  SUBCASE("has value") {
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
    CHECK(buf == b);
  }
}
struct my_test_float {
  float a;
  float b;
  float c;
  float d;
  float e;
};
TEST_CASE("testing float") {
  my_test_float t{.a = 0, .b = 1, .c = -1, .d = 1.234, .e = 1.234e5};
  auto size = get_needed_size(t);
  REQUIRE(size == 20);
  // [20] 15 00 00 80 3f 1d 00 00 80 bf 25 b6 f3 9d 3f 2d 00 04 f1 47
  std::string buf{0x15, 0x00,       0x00,       (char)0x80, 0x3f,
                  0x1d, 0x00,       0x00,       (char)0x80, (char)0xbf,
                  0x25, (char)0xb6, (char)0xf3, (char)0x9d, 0x3f,
                  0x2d, 0x00,       0x04,       (char)0xf1, 0x47};
  auto b = serialize<std::string>(t);
  CHECK(buf == b);
}
TEST_SUITE_END;