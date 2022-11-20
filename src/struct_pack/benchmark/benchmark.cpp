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
#include "../tests/hex_printer.hpp"
#include "config.hpp"
#include "data_def.hpp"
#include "native_pb.hpp"
#include "no_op.h"
#include "protopuf_pb.hpp"
#include "protozero_pb.hpp"
#include "struct_pack/struct_pack.hpp"
#include "struct_pb.hpp"
using pp::operator""_f;
class ScopedTimer {
 public:
  ScopedTimer(const char *name)
      : m_name(name), m_beg(std::chrono::high_resolution_clock::now()) {}
  ScopedTimer(const char *name, uint64_t &ns) : ScopedTimer(name) {
    m_ns = &ns;
  }
  ~ScopedTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto dur =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_beg);
    if (m_ns)
      *m_ns = dur.count();
    else
      std::cout << m_name << " : " << dur.count() << " ns\n";
  }

 private:
  const char *m_name;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_beg;
  uint64_t *m_ns = nullptr;
};


template <typename T>
T get_max() {
  if constexpr (std::is_same_v<T, int8_t>) {
    return 127;
  }
  else if constexpr (std::is_same_v<T, uint8_t>) {
    return 255;
  }
  else if constexpr (std::is_same_v<T, int16_t>) {
    return 32767;
  }
  else if constexpr (std::is_same_v<T, uint16_t>) {
    return 65535;
  }
  else if constexpr (std::is_same_v<T, int32_t>) {
    return INT_MAX - 1;
  }
  else if constexpr (std::is_same_v<T, uint32_t>) {
    return UINT_MAX - 1;
  }
  else if constexpr (std::is_same_v<T, int64_t>) {
    return LLONG_MAX - 1;
  }
  else if constexpr (std::is_same_v<T, uint64_t>) {
    return ULLONG_MAX - 1;
  }

  return {};
}

template <typename T>
inline uint64_t get_avg(const T &v) {
  uint64_t sum = std::accumulate(v.begin(), v.end(), uint64_t(0));
  return sum / v.size();
}

struct tbuffer : public std::vector<char> {
  void write(const char *src, size_t sz) {
    this->resize(this->size() + sz);
    auto pos = this->data() + this->size() - sz;
    std::memcpy(pos, src, sz);
  }
};

std::vector<char> buffer1;
tbuffer buffer2;
std::string buffer3;
std::string buffer4;
using buffer5_t = std::array<std::byte, 3000 * OBJECT_COUNT>;
buffer5_t buffer5;
std::string buffer6;

template <typename T, typename PB, typename BinPB, typename PP_PB>
void bench(T &t, PB &p, BinPB &bin, PP_PB &cmp_pp_pb, std::string tag) {
  buffer1.clear();
  buffer2.clear();
  buffer3.clear();
  buffer4.clear();
  buffer5 = buffer5_t{};

  std::cout << "------- start benchmark " << tag << " -------\n";

  buffer1.reserve(struct_pack::get_needed_size(t));
#ifdef HAVE_MSGPACK
  msgpack::pack(buffer2, t);
  buffer2.reserve(buffer2.size() * SAMPLES_COUNT);
#endif
  auto pb_sz = p.SerializeAsString().size();
  buffer3.reserve(pb_sz * SAMPLES_COUNT);
  buffer4.reserve(struct_pack::pb::get_needed_size(bin));
  // buffer5.reserve(pb_sz);
  buffer6.reserve(pb_sz * SAMPLES_COUNT);

  std::array<std::array<uint64_t, 10>, 12> arr;

  for (int i = 0; i < 10; ++i) {
    buffer1.clear();
    buffer2.clear();
    buffer3.clear();
    buffer4.clear();
    buffer5 = buffer5_t{};
    buffer6.clear();
    {
      ScopedTimer timer("serialize structpack", arr[0][i]);
      for (int j = 0; j < SAMPLES_COUNT; j++) {
        struct_pack::serialize_to(buffer1, t);
      }
      no_op();
    }
#ifdef HAVE_MSGPACK
    {
      ScopedTimer timer("serialize msgpack   ", arr[1][i]);
      for (int j = 0; j < SAMPLES_COUNT; j++) {
        msgpack::pack(buffer2, t);
      }
      no_op();
    }
#endif
    {
      ScopedTimer timer("serialize protobuf", arr[2][i]);
      for (int j = 0; j < SAMPLES_COUNT; j++) {
        buffer3.resize(buffer3.size() + pb_sz);
        p.SerializeToArray(buffer3.data() + buffer3.size() - pb_sz, pb_sz);
      }
      no_op();
    }
    {
      ScopedTimer timer("serialize struct pb", arr[6][i]);
      for (int j = 0; j < SAMPLES_COUNT; j++) {
        struct_pack::pb::serialize_to(buffer4, bin);
      }
      no_op();
    }
    assert(buffer3 == buffer4);
    {
      ScopedTimer timer("serialize pp    pb", arr[8][i]);
      for (int j = 0; j < SAMPLES_COUNT; j++) {
        pp::message_coder<PP_PB>::encode(cmp_pp_pb, buffer5);
      }
      no_op();
    }
    {
      ScopedTimer timer("serialize protozero pb", arr[10][i]);
      for (int j = 0; j < SAMPLES_COUNT; j++) {
        protozero::pbf_writer writer{buffer6};
        protozero_pb::serialize(t, writer);
      }
      no_op();
    }
  }
#ifdef HAVE_MSGPACK
  msgpack::unpacked unpacked;
#endif
  for (int i = 0; i < 10; ++i) {
    {
      ScopedTimer timer("deserialize structpack", arr[3][i]);
      std::size_t len = 0;
      for (size_t j = 0; j < SAMPLES_COUNT; j++) {
        auto ec = struct_pack::deserialize_to_with_offset(t, buffer1, len);
        if (ec != std::errc{}) [[unlikely]] {
          exit(1);
        }
        no_op();
      }
    }
#ifdef HAVE_MSGPACK
    {
      ScopedTimer timer("deserialize msgpack   ", arr[4][i]);
      for (size_t pos = 0, j = 0; j < SAMPLES_COUNT; j++) {
        msgpack::unpack(unpacked, buffer2.data(), buffer2.size(), pos);
        t = unpacked.get().as<T>();
        no_op();
      }
    }
#endif
    {
      ScopedTimer timer("deserialize protobuf", arr[5][i]);
      for (size_t pos = 0, j = 0; j < SAMPLES_COUNT; j++) {
        if (!(p.ParseFromArray(buffer3.data() + pos, pb_sz))) [[unlikely]] {
          exit(1);
        }
        pos += pb_sz;
        no_op();
      }
    }
    {
      ScopedTimer timer("deserialize struct_pb", arr[7][i]);
      std::size_t len = 0;
      std::size_t pos = 0;
      for (size_t j = 0; j < SAMPLES_COUNT; j++) {
        auto ret = struct_pack::pb::deserialize<BinPB>(buffer4.data() + pos,
                                                       pb_sz, len);
        assert(len == pb_sz);
        if (!ret) [[unlikely]] {
          exit(1);
        }
        pos += pb_sz;
        no_op();
        assert(ret.template value() == bin);
      }
    }
    {
      ScopedTimer timer("deserialize pp    pb", arr[9][i]);
      std::size_t len = 0;
      for (size_t j = 0; j < SAMPLES_COUNT; j++) {
        pp::message_coder<PP_PB>::decode(buffer5);
        no_op();
      }
    }
    {
      ScopedTimer timer("deserialize protozero pb", arr[11][i]);
      std::size_t len = 0;
      for (size_t j = 0; j < SAMPLES_COUNT; j++) {
        T tmp_t{};
        protozero::pbf_reader reader{buffer4.data() + 0 * pb_sz, pb_sz};
        protozero_pb::deserialize(tmp_t, reader);
        no_op();
      }
    }
  }

  std::cout << tag << " "
            << "struct_pack serialize average: " << get_avg(arr[0])
            << ", deserialize average:  " << get_avg(arr[3])
            << ", buf size: " << buffer1.size() / SAMPLES_COUNT << "\n";
#ifdef HAVE_MSGPACK
  std::cout << tag << " "
            << "msgpack     serialize average: " << get_avg(arr[1])
            << ", deserialize average: " << get_avg(arr[4])
            << ", buf size: " << buffer2.size() / SAMPLES_COUNT << "\n";
#endif
  std::cout << tag << " "
            << "protobuf    serialize average: " << get_avg(arr[2])
            << ", deserialize average: " << get_avg(arr[5])
            << ", buf size: " << buffer3.size() / SAMPLES_COUNT << "\n";
  std::cout << tag << " "
            << "struct_pb   serialize average: " << get_avg(arr[6])
            << ", deserialize average: " << get_avg(arr[7])
            << ", buf size: " << buffer4.size() / SAMPLES_COUNT << "\n";
  std::cout << tag << " "
            << "protopuf    serialize average: " << get_avg(arr[8])
            << ", deserialize average: " << get_avg(arr[9]) << "\n";
  std::cout << tag << " "
            << "protozero   serialize average: " << get_avg(arr[10])
            << ", deserialize average: " << get_avg(arr[11]) << "\n";
  //<< ", buf size: " << buffer4.size() / SAMPLES_COUNT << "\n";
#ifdef HAVE_MSGPACK
  std::cout << tag << " "
            << "struct_pack serialize is   "
            << (double)get_avg(arr[1]) / get_avg(arr[0])
            << " times faster than msgpack\n";
#endif
  std::cout << tag << " "
            << "struct_pack serialize is   "
            << (double)get_avg(arr[2]) / get_avg(arr[0])
            << " times faster than protobuf\n";
  std::cout << tag << " "
            << "struct_pb   serialize is   "
            << (double)get_avg(arr[2]) / get_avg(arr[6])
            << " times faster than protobuf\n";
  std::cout << tag << " "
            << "struct_pb   serialize is   "
            << (double)get_avg(arr[8]) / get_avg(arr[6])
            << " times faster than protopuf\n";
  std::cout << tag << " "
            << "struct_pb   serialize is   "
            << (double)get_avg(arr[10]) / get_avg(arr[6])
            << " times faster than protozero\n";
#ifdef HAVE_MSGPACK
  std::cout << tag << " "
            << "struct_pack deserialize is "
            << (double)get_avg(arr[4]) / get_avg(arr[3])
            << " times faster than msgpack\n";
#endif
  std::cout << tag << " "
            << "struct_pack deserialize is "
            << (double)get_avg(arr[5]) / get_avg(arr[3])
            << " times faster than protobuf\n";
  std::cout << tag << " "
            << "struct_pb   deserialize is "
            << (double)get_avg(arr[5]) / get_avg(arr[7])
            << " times faster than protobuf\n";
  std::cout << tag << " "
            << "struct_pb   deserialize is "
            << (double)get_avg(arr[9]) / get_avg(arr[7])
            << " times faster than protopuf\n";
  std::cout << tag << " "
            << "struct_pb   deserialize is "
            << (double)get_avg(arr[11]) / get_avg(arr[7])
            << " times faster than protozero\n";
  std::cout << "------- end benchmark   " << tag << " -------\n\n";
}

auto create_rects(size_t object_count) {
  rect<int32_t> rc{65536, 65536, 65536, 65536};
  std::vector<rect<int32_t>> v{};
  for (int i = 0; i < object_count; i++) {
    v.push_back(rc);
  }
  return v;
}

auto create_persons(size_t object_count) {
  std::vector<person> v{};
  person p{65536, "tom", 65536, 65536.42};
  for (int i = 0; i < object_count; i++) {
    v.push_back(p);
  }
  return v;
}

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

auto v = create_rects(OBJECT_COUNT);
auto pbs = create_rects_pb(OBJECT_COUNT);
auto pb = *(pbs.rect32_list().begin());
auto bin_pbs = struct_pb::create_rects(OBJECT_COUNT);
auto bin_pb = bin_pbs.rect32_list[0];
auto cmp_pp_pbs = pp_pb::create_rects(OBJECT_COUNT);
auto cmp_pp_pb = cmp_pp_pbs["rect32_list"_f][0];

auto v1 = create_persons(OBJECT_COUNT);
auto pbs1 = create_persons_pb(OBJECT_COUNT);
auto pb1 = *(pbs1.person_list().begin());
auto bin_pbs1 = struct_pb::create_persons(OBJECT_COUNT);
auto bin_pb1 = bin_pbs1.person_list[0];
auto cmp_pp_pbs1 = pp_pb::create_persons(OBJECT_COUNT);
auto cmp_pp_pb1 = cmp_pp_pbs1["person_list"_f][0];

auto v2 = create_monsters(OBJECT_COUNT);
auto pbs2 = create_monsters_pb(OBJECT_COUNT);
auto pb2 = *(pbs2.monsters().begin());
auto bin_pbs2 = struct_pb::create_monsters(OBJECT_COUNT);
auto bin_pb2 = bin_pbs2.monsters[0];
auto cmp_pp_pbs2 = pp_pb::create_monsters(OBJECT_COUNT);
auto cmp_pp_pb2 = cmp_pp_pbs2["monsters"_f][0];

int main() {
  //  {
  //    bench(v[0], pb, bin_pb, cmp_pp_pb, "1 rect");
  //    bench(v, pbs, bin_pbs, cmp_pp_pbs, "20 rect");
  //  }
  //
  //  {
  //    bench(v1[0], pb1, bin_pb1, cmp_pp_pb1, "1 person");
  //    bench(v1, pbs1, bin_pbs1, cmp_pp_pbs1, "20 person");
  //  }

  {
    bench(v2[0], pb2, bin_pb2, cmp_pp_pb2, "1 monster");
    bench(v2, pbs2, bin_pbs2, cmp_pp_pbs2, "20 monster");
  }
  return 0;
}
