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
struct test3 {
  test1 c;
};
TEST_CASE("testing test1") {
  test1 t;
  t.a = 150;
  auto size = get_needed_size(t);
  REQUIRE(size == 3);
}
TEST_CASE("testing test2") {
  test2 t;
  t.b = "testing";
  auto size = get_needed_size(t);
  REQUIRE(size == 9);
}
TEST_CASE("testing test3") {
  test3 t;
  t.c.a = 150;
  auto size = get_needed_size(t);
  REQUIRE(size == 5);
}