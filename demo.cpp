#include <cassert>
#include <unordered_map>
#include <map>

#include "struct_pack/struct_pack.hpp"
#include "struct_pack/struct_pack/pb.hpp"
#include "demo.pb.h"
#include "src/struct_pack/tests/hex_printer.hpp"
#include "src/struct_pack/benchmark/protopuf_pb.hpp"
#include "src/struct_pack/benchmark/protozero_pb.hpp"
#include "src/struct_pack/benchmark/native_pb.hpp"
#include "src/struct_pack/benchmark/struct_pb.hpp"
#include "src/struct_pack/benchmark/config.hpp"
using namespace struct_pack::pb;

std::vector<Monster> create_monsters(size_t object_count) {
  std::vector<Monster> v{};
  Monster m = {
      .pos = {1, 2, 3},
      .mana = 16,
      .hp = 24,
      .name = "it is a test",
      .inventory = "\1\2\3\4",
      .color = Color::Red,
      .weapons = {{"gun", 42}, {"mission", 56}},
      .equipped = {"air craft", 67},
      .path = {{7, 8, 9}},
  };

  Monster m1 = {
      .pos = {11, 22, 33},
      .mana = 161,
      .hp = 241,
      .name = "it is a test, ok",
      .inventory = "\24\25\26\24",
      .color = Color::Red,
      .weapons = {{"gun", 421}, {"mission", 561}},
      .equipped = {"air craft", 671},
      .path = {{71, 82, 93}},
  };

  for (int i = 0; i < object_count / 2; i++) {
    v.push_back(m);
    v.push_back(m1);
  }

  return v;
}

int main666() {
  auto ms = struct_pb::create_monsters(OBJECT_COUNT);
  auto buf = struct_pack::pb::serialize(ms);
  std::size_t len = 0;
  struct_pack::pb::deserialize<struct_pb::Monsters>(buf.data(), buf.size(), len);
  return 0;
}

int main2() {
  auto ms_pb = create_monsters_pb(20);
  auto m_pb = *(ms_pb.monsters().begin());
  auto ms = create_monsters(20);
  auto m = ms[0];
  auto buf = m_pb.SerializeAsString();
  std::string buffer;
  protozero::pbf_writer pbf_writer{buffer};
  protozero_pb::serialize(m, pbf_writer);
  std::cout << (buf == buffer) << std::endl;
  std::cout << hex_helper(buf) << std::endl;
  std::cout << hex_helper(buffer) << std::endl;
  mygame::Monster pb_d_m;
  pb_d_m.ParseFromArray(buffer.data(), buffer.size());
//  std::cout << (m_pb == pb_d_m) << std::endl;
//  protozero_pb::Monsters d_m{};
//  protozero_pb::deserialize(d_m, buf);
//  std::cout << (ms.size() == d_m.monsters.size()) << std::endl;
//  for (int i = 0; i < ms.size(); ++i) {
//    std::cout << i << "-th: " << (ms[i] == d_m.monsters[i]) << std::endl;
//  }
  return 0;
}
//int main3() {
//  mygame::Monster m;
//  m.set_inventory("\24\25\26\24");
//  auto buf = m.SerializeAsString();
//  Monster d_m;
////  protozero_pb::Monsters d_m{};
//  protozero::pbf_reader reader{buf};
//  protozero_pb::deserialize(d_m, reader);
//  return 0;
//}

int main222() {
  std::variant<std::string, std::string> v{std::in_place_index<1>, std::string("666")};

//  std::holds_alternative<std::string>(v);
  std::string vv = std::get<0>(v);
  std::cout << vv << std::endl;
  return 0;
}
struct sub_message_for_oneof {
  bool ok;
};
struct sample_message_oneof {
  std::variant<varint32_t, varint32_t, std::string, sub_message_for_oneof> t;
};
template<>
constexpr oneof_field_number_array_t<sample_message_oneof, 0>
    oneof_field_number_seq<sample_message_oneof, 0>{
        10, 8, 4, 9
    };
int main5555() {
//  std::variant<std::string, std::string> v;
//  std::cout << v.index() << std::endl;
//  std::cout << std::get<0>(v) << std::endl;
  auto n2i_map = get_field_n2i_map<sample_message_oneof>();
  for (auto [k, v]:n2i_map) {
    std::cout << k << " " << v << std::endl;
  }

  sample_message_oneof t;
  //  t.t = std::variant<varint32_t, varint32_t, std::string, sub_message_for_oneof>{std::in_place_index<3>, "oneof"};
  //  t.t.emplace(std::in_place_index<3>, std::string("oneof"));
  //  t.t.emplace<2>(std::string("oneof"));
  t.t.emplace<3>(sub_message_for_oneof{.ok = true});
  //  std::cout << t.t.index() << std::endl;
  auto size = get_needed_size(t);
  std::cout << "size: " << size << std::endl;
//  std::cout << std::boolalpha << (size == pb_buf.size()) << std::endl;
  auto buf = serialize<std::string>(t);
  std::cout << hex_helper(buf) << std::endl;
  std::size_t len = 0;
//  using T = std::variant_alternative_t<3, decltype(t.t)>;
//  static_assert(std::same_as<T, std::string>);
  auto d_t_ret = deserialize<sample_message_oneof>(buf.data(), buf.size(), len);
//  std::cout << std::boolalpha << bool(d_t_ret) << std::endl;
//  REQUIRE(d_t_ret);
//  REQUIRE(len == buf.size());
//  auto d_t = d_t_ret.value();
//  std::cout << d_t.t.index() << std::endl;
  return 0;
}

enum class A {
  a = 0,
  b = 1,
  c = 1
};

void test_enum() {
  A a{};
  a = A::b;
  std::cout << static_cast<int>(a) << std::endl;
  int t = 100;
  a = static_cast<A>(t);
  std::cout << "= b? " << std::boolalpha << (a == A::b) << std::endl;
  std::cout << "= c? " << std::boolalpha << (a == A::c) << std::endl;
  std::cout << "b = c? " << (A::b == A::c) << std::endl;
}
struct AA {
  struct BB {
    struct CC {
      int c = 0;
    };
  };
};
int main() {
  AA::BB::CC c;
  test_enum();
  return 0;
}
//struct OneofDemo {
//  int a;
//   a = 1;
//   b = 2;
//};
// M(a = 1)

//struct A {
//
//};
//
//struct B:public A {
//
//};