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
  varint32_t a;
};
struct test2 {
  std::string b;
};
template <>
constexpr std::size_t first_field_number<test2> = 2;
struct test3 {
  test1 c;
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
  test1 t;
  t.a = 150;
  auto size = get_needed_size(t);
  REQUIRE(size == 3);
  std::string buf{0x08, (char)0x96, 0x01};
  auto b = serialize<std::string>(t);
  CHECK(buf == b);
}
TEST_CASE("testing test2") {
  test2 t;
  t.b = "testing";
  auto size = get_needed_size(t);
  REQUIRE(size == 9);
  std::string buf{0x12, 0x07};
  buf += t.b;
  auto b = serialize<std::string>(t);
  CHECK(buf == b);
}
TEST_CASE("testing test3") {
  test3 t;
  t.c.a = 150;
  auto size = get_needed_size(t);
  REQUIRE(size == 5);
  std::string buf{0x1a, 0x03, 0x08, (char)0x96, 0x01};
  auto b = serialize<std::string>(t);
  CHECK(buf == b);
}
TEST_CASE("testing test4") {
  test4 t;
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
TEST_SUITE_END;