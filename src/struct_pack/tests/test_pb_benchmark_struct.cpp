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
#include "../benchmark/config.hpp"
#include "../benchmark/native_pb.hpp"
#include "../benchmark/struct_pb.hpp"
#include "doctest.h"
#include "struct_pack/struct_pack/pb.hpp"
using namespace doctest;
using namespace struct_pack::pb;
bool operator==(const std::optional<varint32_t>& a, int32_t b) {
  return int32_t(a.value()) == b;
}
bool operator==(const std::optional<struct_pb::Vec3>& a,
                const mygame::Vec3& b) {
  assert(a.has_value());
  auto val = a.value();
  return val.x == b.x() && val.y == b.y() && val.z == b.z();
}
bool operator==(const std::optional<struct_pb::Monster::Color>& a,
                const mygame::Monster::Color& b) {
  assert(a.has_value());
  auto val = a.value();
  return int(val) == int(b);
}

bool operator==(const std::optional<struct_pb::Weapon>& a,
                const mygame::Weapon& b) {
  assert(a->name.has_value());
  assert(a->damage.has_value());
  return a->name == b.name() && a->damage == b.damage();
}

template <typename T, typename U>
bool operator==(const std::vector<T>& a,
                const google::protobuf::RepeatedPtrField<U>& b) {
  if (a.size() != b.size()) {
    return false;
  }
  auto sz = a.size();
  for (int i = 0; i < sz; ++i) {
    if (a[i] != b.at(i)) {
      return false;
    }
  }
  return true;
}
template <typename T, typename U>
void check_repeated(const std::vector<T>& a,
                    const google::protobuf::RepeatedPtrField<U>& b) {
  bool ret = (a == b);
  REQUIRE(ret);
}
// useful for debugging
void print_hex(uint8_t v) {
  static std::string hex = "0123456789abcedf";
  if (v < 16) {
    std::cout << "0";
    std::cout << hex[v];
  }
  else {
    auto first = v / 16;
    auto last = v % 16;
    std::cout << hex[first] << hex[last];
  }
}
void print_hex(std::string_view sv) {
  for (char ch : sv) {
    uint8_t v = ch;
    print_hex(v);
    std::cout << " ";
  }
  std::cout << std::endl;
}
TEST_CASE("testing rect") {
  auto my_rects = struct_pb::create_rects(OBJECT_COUNT);
  auto pb_rects = create_rects_pb(OBJECT_COUNT);

  for (int i = 0; i < OBJECT_COUNT; ++i) {
    auto my_rect = my_rects.rect32_list[i];
    auto pb_rect = *(pb_rects.rect32_list().begin() + i);

    REQUIRE(my_rect.x.has_value());
    REQUIRE(my_rect.x == pb_rect.x());
    REQUIRE(my_rect.y.has_value());
    REQUIRE(my_rect.y == pb_rect.y());
    REQUIRE(my_rect.width.has_value());
    REQUIRE(my_rect.width == pb_rect.width());
    REQUIRE(my_rect.height.has_value());
    REQUIRE(my_rect.height == pb_rect.height());

    auto pb_buf = pb_rect.SerializeAsString();
    auto sz = get_needed_size(my_rect);
    REQUIRE(sz == pb_buf.size());
    CHECK(sz == 16);

    auto my_buf = serialize<std::string>(my_rect);
    REQUIRE(my_buf == pb_buf);
  }

  auto pb_buf2 = pb_rects.SerializeAsString();
  auto sz = get_needed_size(my_rects);
  CHECK(sz == pb_buf2.size());

  auto my_buf2 = serialize<std::string>(my_rects);
  REQUIRE(my_buf2.size() == pb_buf2.size());
  REQUIRE(my_buf2 == pb_buf2);
}

TEST_CASE("testing person") {
  auto my_persons = struct_pb::create_persons(OBJECT_COUNT);
  auto pb_persons = create_persons_pb(OBJECT_COUNT);

  for (int i = 0; i < OBJECT_COUNT; ++i) {
    auto my_person = my_persons.person_list[i];
    auto pb_person = *(pb_persons.person_list().begin() + i);
    REQUIRE(my_person.id.has_value());
    REQUIRE(my_person.id == pb_person.id());
    REQUIRE(my_person.name.has_value());
    REQUIRE(my_person.name == pb_person.name());
    REQUIRE(my_person.age.has_value());
    REQUIRE(my_person.age == pb_person.age());
    REQUIRE(my_person.salary.has_value());
    REQUIRE(my_person.salary == pb_person.salary());

    auto pb_buf = pb_person.SerializeAsString();
    auto sz = get_needed_size(my_person);
    REQUIRE(sz == pb_buf.size());
    CHECK(sz == 22);

    auto my_buf = serialize<std::string>(my_person);
    REQUIRE(my_buf == pb_buf);
  }

  auto pb_buf2 = pb_persons.SerializeAsString();
  auto sz = get_needed_size(my_persons);
  CHECK(sz == pb_buf2.size());

  auto my_buf2 = serialize<std::string>(my_persons);
  REQUIRE(my_buf2.size() == pb_buf2.size());
  REQUIRE(my_buf2 == pb_buf2);
}

TEST_CASE("testing monsters") {
  auto my_ms = struct_pb::create_monsters(OBJECT_COUNT);
  auto pb_ms = create_monsters_pb(OBJECT_COUNT);

  for (int i = 0; i < OBJECT_COUNT; ++i) {
    auto my_m = my_ms.monsters[i];
    auto pb_m = *(pb_ms.monsters().begin() + i);

    REQUIRE(my_m.pos.has_value());
    REQUIRE(my_m.pos == pb_m.pos());
    REQUIRE(my_m.mana.has_value());
    REQUIRE(my_m.mana == pb_m.mana());
    REQUIRE(my_m.hp.has_value());
    REQUIRE(my_m.hp == pb_m.hp());
    REQUIRE(my_m.name.has_value());
    REQUIRE(my_m.name == pb_m.name());
    REQUIRE(my_m.inventory.has_value());
    REQUIRE(my_m.inventory == pb_m.inventory());
    REQUIRE(my_m.color.has_value());
    REQUIRE(my_m.color == pb_m.color());
    // REQUIRE(my_m.weapons == pb_m.weapons());
    check_repeated(my_m.weapons, pb_m.weapons());
    check_repeated(my_m.path, pb_m.path());

    auto pb_buf = pb_m.SerializeAsString();
    auto sz = get_needed_size(my_m);
    REQUIRE(sz == pb_buf.size());

    auto my_buf = serialize<std::string>(my_m);
    REQUIRE(my_buf == pb_buf);
  }

  auto pb_buf2 = pb_ms.SerializeAsString();
  auto sz = get_needed_size(my_ms);
  CHECK(sz == pb_buf2.size());

  auto my_buf2 = serialize<std::string>(my_ms);
  REQUIRE(my_buf2.size() == pb_buf2.size());
  REQUIRE(my_buf2 == pb_buf2);
}
TEST_CASE("testing debugging ") {
  struct_pb::Monster my_m;
  mygame::Monster pb_m;

  my_m.pos = {1, 2, 3};
  auto vec = new mygame::Vec3;
  vec->set_x(1);
  vec->set_y(2);
  vec->set_z(3);
  pb_m.set_allocated_pos(vec);

  my_m.mana = 16;
  my_m.hp = 24;
  my_m.name = "it is a test";
  my_m.inventory = "\1\2\3\4";

  pb_m.set_mana(16);
  pb_m.set_hp(24);
  pb_m.set_name("it is a test");
  pb_m.set_inventory("\1\2\3\4");

  my_m.color = struct_pb::Monster::Color::Red;
  pb_m.set_color(mygame::Monster_Color::Monster_Color_Red);

  auto pb_buf = pb_m.SerializeAsString();
  auto sz = get_needed_size(my_m);
  REQUIRE(sz == pb_buf.size());

  auto my_buf = serialize<std::string>(my_m);
  REQUIRE(my_buf == pb_buf);
}