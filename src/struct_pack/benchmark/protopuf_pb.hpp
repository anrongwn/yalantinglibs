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
#include "protopuf/message.h"
namespace pp_pb {
using namespace pp;

using Vec3 =
    message<float_field<"x", 1>, float_field<"y", 2>, float_field<"z", 3> >;
using Weapon = message<string_field<"name", 1>, int32_field<"damage", 2> >;
enum class Color {
  Red = 0,
  Green = 1,
  Blue = 2,
};
using Monster =
    message<message_field<"pos", 1, Vec3>, int32_field<"mana", 2>,
            int32_field<"hp", 3>, string_field<"name", 4>,
            bytes_field<"inventory", 5>, enum_field<"color", 6, Color>,
            message_field<"weapons", 7, Weapon, repeated>,
            message_field<"equipped", 8, Weapon>,
            message_field<"path", 9, Vec3, repeated> >;
using Monsters = message<message_field<"monsters", 1, Monster, repeated> >;
using rect32 = message<int32_field<"x", 1>, int32_field<"y", 2>,
                       int32_field<"width", 3>, int32_field<"height", 4> >;
using rect32s = message<message_field<"rect32_list", 1, rect32, repeated> >;
using person = message<int32_field<"id", 1>, string_field<"name", 2>,
                       int32_field<"age", 3>, double_field<"salary", 4> >;
using persons = message<message_field<"person_list", 1, person, repeated> >;

auto create_rects(std::size_t object_count) {
  rect32s rcs;
  for (int i = 0; i < object_count; ++i) {
    rect32 rc{65536, 65536, 65536, 65536};
    rcs["rect32_list"_f].push_back(rc);
  }
  return rcs;
}
auto create_persons(std::size_t object_count) {
  persons ps;
  for (int i = 0; i < object_count; ++i) {
    person p{65536, "tom", 65536, 65536.42};
    ps["person_list"_f].push_back(p);
  }
  return ps;
}
auto create_monsters(std::size_t object_count) {
  Monsters monsters;
  for (int i = 0; i < object_count / 2; ++i) {
    {
      Monster m;
      m["pos"_f] = Vec3{1, 2, 3};
      m["mana"_f] = 16;
      m["hp"_f] = 24;
      m["name"_f] = "it is a test";
      m["inventory"_f] = std::vector<unsigned char>{'\1', '\2', '\3', '\4'};
      // LLVM crash
      // static_assert(std::same_as<decltype(m["inventory"_f]), int32_t>);
      m["color"_f] = Color::Red;
      m["weapons"_f].emplace_back("gun", 42);
      m["weapons"_f].emplace_back("mission", 56);
      m["equipped"_f] = Weapon{"air craft", 67};
      m["path"_f] = std::vector<Vec3>{Vec3{7, 8, 9}};
      monsters["monsters"_f].push_back(m);
    }
    {
      Monster m;
      m["pos"_f] = Vec3{11, 22, 33};
      m["mana"_f] = 161;
      m["hp"_f] = 241;
      m["name"_f] = "it is a test, ok";
      m["inventory"_f] = std::vector<unsigned char>{'\24', '\25', '\26', '\24'};
      m["color"_f] = Color::Red;
      m["weapons"_f].emplace_back("gun", 421);
      m["weapons"_f].emplace_back("mission", 561);
      m["equipped"_f] = Weapon{"air craft", 671};
      m["path"_f] = std::vector<Vec3>{Vec3{71, 82, 93}};
      monsters["monsters"_f].push_back(m);
    }
  }
  return monsters;
}
}  // namespace pp_pb