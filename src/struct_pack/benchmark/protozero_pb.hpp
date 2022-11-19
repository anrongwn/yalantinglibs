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
#include "data_def.hpp"
#include "protozero/pbf_reader.hpp"

namespace protozero_pb {
using namespace protozero;
void deserialize(Vec3& vec3, data_view buffer) {
  pbf_reader message{buffer};
  while (message.next()) {
    switch (message.tag()) {
      case 1:
        vec3.x = message.get_float();
        break;
      case 2:
        vec3.y = message.get_float();
        break;
      case 3:
        vec3.z = message.get_float();
        break;
      default:
        message.skip();
    }
  }
}
void deserialize(Weapon& weapon, data_view buffer) {
  pbf_reader message{buffer};
  while (message.next()) {
    switch (message.tag()) {
      case 1:
        weapon.name = message.get_string();
        break;
      case 2:
        weapon.damage = message.get_int32();
        break;
      default:
        message.skip();
    }
  }
}
void deserialize(Monster& monster, data_view buffer) {
  pbf_reader message{buffer};
  while (message.next()) {
    switch (message.tag()) {
      case 1: {
        auto buf = message.get_view();
        deserialize(monster.pos, buf);
        break;
      }
      case 2:
        monster.mana = message.get_int32();
        break;
      case 3:
        monster.hp = message.get_int32();
        break;
      case 4:
        monster.name = message.get_string();
        break;
      case 5: {
        monster.inventory = message.get_string();
        break;
      }
      case 6:
        monster.color = static_cast<Color>(message.get_enum());
        break;
      case 7: {
        auto buf = message.get_view();
        Weapon w{};
        deserialize(w, buf);
        monster.weapons.push_back(w);
        break;
      }
      case 8: {
        auto buf = message.get_view();
        deserialize(monster.equipped, buf);
        break;
      }
      case 9: {
        auto buf = message.get_view();
        Vec3 v{};
        deserialize(v, buf);
        monster.path.push_back(v);
        break;
      }
    }
  }
}
template <typename T>
void deserialize(std::vector<T>& ms, data_view buffer) {
  pbf_reader message{buffer};
  while (message.next()) {
    switch (message.tag()) {
      case 1: {
        auto buf = message.get_view();
        T m{};
        deserialize(m, buf);
        ms.push_back(m);
        break;
      }
      default: {
        message.skip();
      }
    }
  }
}
}  // namespace protozero_pb