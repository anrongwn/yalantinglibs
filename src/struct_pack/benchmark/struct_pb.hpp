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
#include <optional>
#include <valarray>

#include "struct_pack/struct_pack/pb.hpp"
namespace struct_pb {
struct Vec3 {
  float x;
  float y;
  float z;

  bool operator==(const Vec3 &rhs) const {
    std::valarray<float> lh({x, y, z});
    std::valarray<float> rh({rhs.x, rhs.y, rhs.z});
    return (std::abs(lh - rh) < 0.05f).min();
  };
};
struct Weapon {
  std::string name;
  struct_pack::pb::varint32_t damage;
  bool operator==(const Weapon &rhs) const {
    return name == rhs.name && damage == rhs.damage;
  };
};
struct Monster {
  std::optional<Vec3> pos;
  struct_pack::pb::varint32_t mana;
  struct_pack::pb::varint32_t hp;
  std::string name;
  std::string inventory;
  enum class Color { Red, Green, Blue };
  Color color;
  std::vector<Weapon> weapons;
  std::optional<Weapon> equipped;
  std::vector<Vec3> path;
  bool operator==(const Monster &rhs) const {
    return pos == rhs.pos && mana == rhs.mana && hp == rhs.hp &&
           name == rhs.name && inventory == rhs.inventory &&
           color == rhs.color && weapons == rhs.weapons &&
           equipped == rhs.equipped && path == rhs.path;
  };
};
struct Monsters {
  std::vector<Monster> monsters;
  bool operator==(const Monsters &) const = default;
};
struct rect32 {
  struct_pack::pb::varint32_t x;
  struct_pack::pb::varint32_t y;
  struct_pack::pb::varint32_t width;
  struct_pack::pb::varint32_t height;
  bool operator==(const rect32 &rhs) const {
    return x == rhs.x && y == rhs.y && width == rhs.width &&
           height == rhs.height;
  }
};
struct rect32s {
  std::vector<rect32> rect32_list;
  bool operator==(const rect32s &rhs) const {
    return rect32_list == rhs.rect32_list;
  }
};
struct person {
  struct_pack::pb::varint32_t id;
  std::string name;
  struct_pack::pb::varint32_t age;
  double salary;
  bool operator==(const person &rhs) const {
    return id == rhs.id && name == rhs.name && age == rhs.age &&
           salary == rhs.salary;
  }
};
struct persons {
  std::vector<person> person_list;
  bool operator==(const persons &rhs) const {
    return person_list == rhs.person_list;
  }
};
auto create_rects(std::size_t object_count) {
  rect32s rcs;
  for (int i = 0; i < object_count; ++i) {
    rect32 rc{65536, 65536, 65536, 65536};
    rcs.rect32_list.emplace_back(rc);
  }
  return rcs;
}
auto create_persons(std::size_t object_count) {
  persons ps;
  for (int i = 0; i < object_count; ++i) {
    person p{65536, "tom", 65536, 65536.42};
    ps.person_list.emplace_back(p);
  }
  return ps;
}
auto create_monsters(std::size_t object_count) {
  Monsters monsters;
  for (int i = 0; i < object_count / 2; ++i) {
    {
      Monster m;
      m.pos = {1, 2, 3};
      m.mana = 16;
      m.hp = 24;
      m.name = "it is a test";
      m.inventory = "\1\2\3\4";
      m.color = Monster::Color::Red;
      m.weapons = {{"gun", 42}, {"mission", 56}};
      m.equipped = {"air craft", 67};
      m.path = {{7, 8, 9}};
      monsters.monsters.push_back(m);
    }
    {
      Monster m;
      m.pos = {11, 22, 33};
      m.mana = 161;
      m.hp = 241;
      m.name = "it is a test, ok";
      m.inventory = "\24\25\26\24";
      m.color = Monster::Color::Red;
      m.weapons = {{"gun", 421}, {"mission", 561}};
      m.equipped = {"air craft", 671};
      m.path = {{71, 82, 93}};
      monsters.monsters.push_back(m);
    }
  }
  return monsters;
}
}  // namespace struct_pb