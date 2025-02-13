/*
 * Copyright (c) 2023, Alibaba Group Holding Limited;
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
#include "ylt/struct_pack/struct_pack_impl.hpp"

namespace struct_pack {

/*!
 * \defgroup  struct_pack struct_pack
 *
 * \brief yaLanTingLibs struct_pack 序列化库
 *
 */

/*!
 * \ingroup struct_pack
 * \struct expected
 * \brief deserialize的返回值
 *
 * 当启用C++23标准且标准库已实现std::expected时，该类是std::expected的别名。否则该类将使用内部的实现代码。
 * 例如：
 * ```cpp
 * person p{20, "tom"};
 * auto buffer = struct_pack::serialize(p);
 * auto p2 = struct_pack::deserialize<person>(buffer.data(), buffer.size());
 * assert(p2);
 * assert(p == p2.value());
 * ```
 */

/*!
 * \ingroup struct_pack
 * \struct trivial_view
 * \brief
 * trivial_view<T> 是一个平凡结构体的视图，在类型系统上等价于T。
 * 其作用是减少赋值/反序列化过程中的内存拷贝。
 *
 * 例如，假如有一个巨大的结构体Data
 *
 * ```cpp
 * struct Data {
 *   int x[10000],y[10000],z[10000];
 * };
 *
 * 假设有协议：
 * ```cpp
 * struct Proto {
 *   std::string name;
 *   Data data;
 * };
 * void serialzie(std::string_view name, Data& data) {
 *   Proto proto={.name = name, .data = data};
 *   //从已有的数据构造对象时，需要花费大量时间拷贝
 *   auto buffer = struct_pack::serialize(proto);
 *   auto result = struct_pack::deserialize<Proto>(data);
 *   //反序列化时，需要花费大量时间拷贝
 *   assert(result->name == name && result->data == data);
 * }
 * ```
 *
 * 可以发现，构造/反序列化时拷贝代价很大。
 * 如何解决？我们可以改用视图：
 * ```cpp
 * struct ProtoView {
 *   std::string_view name;
 *   struct_pack::trivial_view<Data> data;
 * };
 * void serialzie(std::string_view name, Data& data) {
 *   ProtoView proto={.name = name, .data = data};
 *   //从已有的数据构造对象时，可以做到zero-copy
 *   auto buffer = struct_pack::serialize(proto);
 *   auto result = struct_pack::deserialize<ProtoView>(data);
 *   //反序列化是zero-copy的。
 *   assert(result->name == name && result->data.get() == data);
 * }
 * ```
 * 由于在类型系统上trivial_view<T>等价于T，故两者在内存布局上等价。
 * 因此，序列化其中一者再反序列化到另外一者是合法的。
 * ```cpp
 * void serialzie(Proto& proto) {
 *   auto buffer = struct_pack::serialize(proto);
 *   auto result = struct_pack::deserialize<ProtoView>(data);
 *   //反序列化是zero-copy的。
 *   assert(result->name == name && result->data.get() == data);
 * }
 * ```
 *
 */

/*!
 * \ingroup struct_pack
 * \struct compatible
 * \brief
 * 这个类使用上类似于std::optional<T>，但其语义是添加一个能够保持向前兼容的字段。
 *
 * 例如：
 *
 * ```cpp
 * struct person_v1 {
 *   int age;
 *   std::string name;
 * };
 *
 * struct person_v2 {
 *   int age;
 *   std::string name;
 *   struct_pack::compatible<std::size_t, 20210101> id; //20210101为版本号
 * };
 * ```
 *
 * `struct_pack::compatible<T, version_number>`
 * 可以为空值，从而保证向前和向后的兼容性。
 * 例如，序列化person_v2，然后将其按照person_v1来反序列化，多余的字段在反序列化时将会被直接忽略。
 * 反过来说，序列化person_v1，再将其按照person_v2来反序列化，则解析出的person_v2中的compatibale字段均为空值。
 *
 * compatible字段的版本号可以省略，默认版本号为0。
 *
 * ```cpp
 * person_v2 p2{20, "tom", 114.1, "tom"};
 * auto buf = struct_pack::serialize(p2);
 *
 * person_v1 p1;
 * // deserialize person_v2 as person_v1
 * auto ec = struct_pack::deserialize_to(p1, buf.data(), buf.size());
 * CHECK(ec == std::errc{});
 *
 * auto buf1 = struct_pack::serialize(p1);
 * person_v2 p3;
 * // deserialize person_v1 as person_v2
 * auto ec = struct_pack::deserialize_to(p3, buf1.data(), buf1.size());
 * CHECK(ec == std::errc{});
 * ```
 *
 * 升级接口时应注意保持 **版本号递增** 原则：
 * 1.
 * 新增加的compatible字段的版本号，应该大于上一次增加的compatible字段的版本号。
 * 2.
 * 不得移除原来的任何一个compatible字段（移除非compatible字段也会出错，但这种错误是可以被类型校验检查出来的）
 *
 * 假如违反这一原则，那么可能会引发未定义行为。
 * 例如：
 *
 * ```cpp
 * struct loginRequest_V1
 * {
 *   string user_name;
 *   string pass_word;
 *   struct_pack::compatible<string> verification_code;
 * };
 *
 * struct loginRequest_V2
 * {
 *   string user_name;
 *   string pass_word;
 *   struct_pack::compatible<network::ip> ip_address;
 * };
 *
 * auto data=struct_pack::serialize(loginRequest_V1{});
 * loginRequest_V2 req;
 * //该行为是未定义的！
 * struct_pack::deserialize_to(req, data);
 *
 * ```
 *
 * ```cpp
 * struct loginRequest_V1
 * {
 *   string user_name;
 *   string pass_word;
 *   struct_pack::compatible<string, 20210101> verification_code;
 * };
 *
 * struct loginRequest_V2
 * {
 *   string user_name;
 *   string pass_word;
 *   struct_pack::compatible<string, 20210101> verification_code;
 *   struct_pack::compatible<network::ip, 20000101> ip_address;
 * };
 *
 * auto data=struct_pack::serialize(loginRequest_V1{});
 * loginRequest_V2 req;
 * //该行为是未定义的！
 * struct_pack::deserialize_to(req, data);
 *
 * ```
 *
 */

/*!
 * \ingroup struct_pack
 * \struct string_literal
 * \brief
 * 该类用于表示一个编译期的字符串类型，是函数`struct_pack::get_type_literal`的返回值。
 *
 *
 * 样例代码：
 *
 * ```cpp
 * auto str = get_type_literal<int, int, short>();
 * CHECK(str.size() == 5);
 * string_literal<char, 5> val{{(char)-2, 1, 1, 7, (char)-1}};
 * //{struct_begin,int32_t,int32_t,int16_t,struct_end}; CHECK(str == val);
 * ```
 */

/*!
 * \ingroup struct_pack
 * \struct members_count
 * \brief
 * 某些特殊情况下，struct_pack可能无法正确计算结构体内的元素个数并导致编译期错误。
 * 此时请特化模板`struct_pack::members_count`，并手动标明结构体内元素的个数。
 *
 * 样例代码：
 * ```cpp
 * struct bug_member_count_struct1 {
 *  int i;
 *  std::string j;
 *  double k = 3;
 * };
 *
 * struct bug_member_count_struct2 : bug_member_count_struct1 {};
 *
 * template <>
 * constexpr size_t struct_pack::members_count<bug_member_count_struct2> = 3;
 * ```
 *
 */

/*!
 * \ingroup struct_pack
 * \struct members_count
 * \brief
 * 某些特殊情况下，struct_pack可能无法正确计算结构体内的元素个数并导致编译期错误。
 * 此时请特化模板`struct_pack::members_count`，并手动标明结构体内元素的个数。
 *
 * 样例代码：
 * ```cpp
 * struct bug_member_count_struct1 {
 *  int i;
 *  std::string j;
 *  double k = 3;
 * };
 *
 * struct bug_member_count_struct2 : bug_member_count_struct1 {};
 *
 * template <>
 * constexpr size_t struct_pack::members_count<bug_member_count_struct2> = 3;
 * ```
 *
 */

/*!
 * \ingroup struct_pack
 * 本函数返回std::errc对应的错误消息。
 * @param err
 * @return
 */
STRUCT_PACK_INLINE std::string error_message(struct_pack::errc err);

/*!
 * \ingroup struct_pack
 * 用于预先分配好合适长度的内存，通常配合`struct_pack::serialize_to`函数使用。
 * 如果类型允许，该计算可能在编译期完成。
 *
 * 样例代码：
 *
 * ```cpp
 * std::map<std::string, std::string> obj{{"hello", "struct pack"}};
 * auto size=struct_pack::get_needed_size(obj);
 * auto array=std::make_unique<char[]>(size);
 * struct_pack::serialize_to(array.get(),size);
 * ```
 *
 * @tparam Args
 * @param args
 * @return 序列化所需的buffer的长度。
 */
template <typename... Args>
[[nodiscard]] STRUCT_PACK_INLINE constexpr auto get_needed_size(
    const Args &...args);

/*!
 * \ingroup struct_pack
 * 返回一个31位的类型T的MD5校验码。当传入的参数多于一个时，返回类型`std::tuple<Args...>`的校验码
 *
 * 样例代码：
 *
 * ```cpp
 *     static_assert(
 *         get_type_code<std::deque<int>>() ==
 * get_type_code<std::vector<int>>(), "different class accord with container
 * concept should get same MD5");
 * ```
 *
 * @tparam Args
 * @return 编译期计算出的类型的哈希校验码。
 */
template <typename... Args>
STRUCT_PACK_INLINE consteval std::uint32_t get_type_code();

/*!
 * \ingroup struct_pack
 * 本函数返回编译期计算出的类型名，并按`struct_pack::string_literal<char,N>`类型返回。
 * 当传入的参数多于一个时，返回类型`std::tuple<T...>`的类型名。
 *
 * 样例代码：
 *
 * ```cpp
 * auto str = get_type_literal<tuple<int, int, short>>();
 * CHECK(str.size() == 5);
 * string_literal<char, 5> val{{(char)-2, 1, 1, 7, (char)-1}};
 * //{struct_begin,int32_t,int32_t,int16_t,struct_end}; CHECK(str == val);
 * ```
 * @tparam Args
 * @return 编译期计算出的类型名
 */
template <typename... Args>
STRUCT_PACK_INLINE consteval decltype(auto) get_type_literal();

/*!
 * \ingroup struct_pack
 * 序列化
 *
 * 样例代码
 * ```
 * std::map<std::string, std::string> obj{{"hello", "struct pack"}};
 * // 序列化对象
 * {
 *   auto buffer=struct_pack::serialize(obj);
 * }
 * // 保存到std::string
 * {
 *   auto buffer=struct_pack::serialize<std::string>(obj);
 * }
 * // 打包序列化多个对象
 * {
 *   auto buffer=struct_pack::serialize(obj,42,"Hello world");
 * }
 * ```
 *
 * @tparam Buffer 传入的自定义`Buffer`类型需要满足以下约束条件：
 * 1. 是顺序容器，具有成员函数begin(),end(),size(),data(),resize()等函数;
 * 2. 容器的value_type为char,unsigned char 或者 std::byte
 * 3. 容器的内存布局是连续的
 * @tparam Args
 * @param args args为需要序列化的对象。
 *             当传入多个序列化对象时，函数会将其打包合并，按std::tuple类型的格式序列化。
 * @return
 * 返回保存在容器中的序列化结果，容器类型为模板参数中的buffer，默认为std::vector<char>。
 *         也支持std::string或者自定义容器
 */
template <detail::struct_pack_buffer Buffer = std::vector<char>,
          typename... Args>
[[nodiscard]] STRUCT_PACK_INLINE Buffer serialize(const Args &...args);

/*!
 * \ingroup struct_pack
 * 该函数在 struct_pack::serialize_with_offset 的基础上，增加了一个offset参数。
 *
 * 异常：struct_pack不主动抛出异常。
 *
 * 样例代码：
 * ```cpp
 * std::map<std::string, std::string> obj{{"hello", "struct pack"}};
 * // 保存到std::string, 并且在string的头部预留一个int的空间。
 * {
 *   auto
 * buffer=struct_pack::serialize_with_offset<std::string>(sizeof(int),obj);
 * }
 * ```
 *
 * @tparam Buffer 传入的自定义`Buffer`类型需要满足以下约束条件：
 * 1. 是顺序容器，具有成员函数begin(),end(),size(),data(),resize()等函数;
 * 2. 容器的value_type为char,unsigned char 或者 std::byte
 * 3. 容器的内存布局是连续的
 * @tparam Args
 * @param offset 序列化结果的头部预留的空字节数。
 * @param args args为需要序列化的对象。
 *             当传入多个序列化对象时，函数会将其打包合并，按std::tuple类型的格式序列化。
 * @return
 * 返回保存在容器中的序列化结果，容器类型为模板参数中的buffer，默认为std::vector<char>。
 *         也支持std::string或者自定义容器。
 */
template <detail::struct_pack_buffer Buffer = std::vector<char>,
          typename... Args>
[[nodiscard]] STRUCT_PACK_INLINE Buffer
serialize_with_offset(std::size_t offset, const Args &...args);

/*!
 * \ingroup struct_pack
 * @tparam Byte
 * @tparam Args args为需要序列化的对象。
 *             当传入多个序列化对象时，函数会将其打包合并，按std::tuple类型的格式序列化。
 * @param buffer 内存首地址
 * @param len 该段内存的长度
 * @param args 待序列化的对象
 * @return
 * 返回序列化结果所占的长度。当序列化结果所需的内存大于len时，不进行序列化，并返回0。
 */
template <detail::struct_pack_byte Byte, typename... Args>
std::size_t STRUCT_PACK_INLINE serialize_to(Byte *buffer, std::size_t len,
                                            const Args &...args) noexcept;

/*!
 * \ingroup struct_pack
 * 将序列化结果添加到已有的容器尾部，或者保存到一块连续的内存中。
 * 样例代码
 * ```cpp
 * std::map<std::string, std::string> obj{{"hello", "struct pack"}};
 * // 序列化到已有的容器末尾
 * {
 *   auto buffer=serialize(42);
 *   struct_pack::serialize_to(buffer, obj);
 * }
 * // 序列化到某段内存中
 * {
 *   auto size=struct_pack::get_needed_size(obj);
 *   auto array=std::make_unique<char[]>(size);
 *   struct_pack::serialize_to(array.get(),size);
 * }
 * // 内存长度不足
 * {
 *   std::array<char,4> ar;
 *   auto len=struct_pack::serialize_to(ar,obj);
 *   assert(len==0); //数组长度不足！
 * }
 * ```
 * @tparam Buffer
 * @tparam Args args为需要序列化的对象。
 *             当传入多个序列化对象时，函数会将其打包合并，按std::tuple类型的格式序列化。
 * @param buffer 保存序列化结果的容器
 * @param args 待序列化的对象
 */
template <detail::struct_pack_buffer Buffer, typename... Args>
void STRUCT_PACK_INLINE serialize_to(Buffer &buffer, const Args &...args);

/*!
 * \ingroup struct_pack
 * 将序列化结果添加到已有的容器尾部，并在序列化结果的头部添加一段空白字节。
 *
 * @tparam Buffer
 * @tparam Args
 * @param buffer 保存序列化结果的容器
 * @param offset 序列化结果的头部预留的空字节数。
 * @param args 待序列化的对象
 */
template <detail::struct_pack_buffer Buffer, typename... Args>
void STRUCT_PACK_INLINE serialize_to_with_offset(Buffer &buffer,
                                                 std::size_t offset,
                                                 const Args &...args);

/*!
 *
 * 样例代码：
 *
 * ```cpp
 * person p{20, "tom"};
 * std::string buffer;
 * auto
 * buffer=struct_pack::serialize_to_with_offset(buffer,sizeof(std::size_t),p);
 * std::size_t sz=buffer.size()-sizeof(std::size_t);
 * memcpy(buffer.data(),&sz,sizeof(std::size_t));
 * //then send it to tcp data flow
 * ```
 *
 * @tparam T 反序列化的类型，需要用户手动填写
 * @tparam Args 函数允许填入多个类型参数，然后按std::tuple类型反序列化
 * @tparam Byte
 * @param data 指向保存序列化结果的内存首地址。
 * @param size 内存的长度
 * @return 当用户只填入一个类型参数时，返回struct_pack::expected<T,
 * std::errc>类型，该expected类型包含反序列化结果T或std::errc类型的错误码。
 * 否则当用户填入多个类型参数时，返回struct_pack::expected<std::tuple<T,Args...>,std::errc>类型
 */
template <typename T, typename... Args, detail::struct_pack_byte Byte>
[[nodiscard]] STRUCT_PACK_INLINE auto deserialize(const Byte *data,
                                                  size_t size);

/*!
 * \ingroup struct_pack
 * @tparam T 反序列化的类型，需要用户手动填写
 * @tparam Args 函数允许填入多个类型参数，然后按std::tuple类型反序列化
 * @tparam View
 * @param v 待反序列化的数据，其包含成员函数data()和size()。
 * @return 当用户只填入一个类型参数时，返回struct_pack::expected<T,
 * std::errc>类型，该expected类型包含反序列化结果T或std::errc类型的错误码。
 * 否则当用户填入多个类型参数时，返回struct_pack::expected<std::tuple<T,Args...>,std::errc>类型
 */
template <typename T, detail::deserialize_view View>
[[nodiscard]] STRUCT_PACK_INLINE auto deserialize(const View &v);

/*!
 * \ingroup struct_pack
 * 样例代码：
 *
 * ```cpp
 * std::string hi = "Hello struct_pack";
 * std::vector<char> ho;
 * for (int i = 0; i < 100; ++i) {
 *   struct_pack::serialize_to(ho, hi + std::to_string(i));
 * }
 * for (size_t i = 0, offset = 0; i < 100; ++i) {
 *   auto ret = struct_pack::deserialize_with_offset<std::string>(ho, offset);
 *   CHECK(ret.errc == std::errc{});
 *   CHECK(ret.value == hi + std::to_string(i));
 * }
 * ```
 *
 * @tparam T 反序列化的类型，需要用户手动填写
 * @tparam Args 函数允许填入多个类型参数，然后按std::tuple类型反序列化
 * @tparam Byte
 * @param data 指向保存序列化结果的内存首地址。
 * @param size 该段内存的长度
 * @param offset 反序列化起始位置的偏移量
 * @return 当用户只填入一个类型参数时，返回struct_pack::expected<T,
 * std::errc>类型，该expected类型包含反序列化结果T或std::errc类型的错误码。
 * 否则当用户填入多个类型参数时，返回struct_pack::expected<std::tuple<T,Args...>,std::errc>类型
 */
template <typename T, detail::struct_pack_byte Byte>
[[nodiscard]] STRUCT_PACK_INLINE auto deserialize_with_offset(const Byte *data,
                                                              size_t size,
                                                              size_t &offset);

/*!
 * \ingroup struct_pack
 * @tparam T 反序列化的类型，需要用户手动填写
 * @tparam Args 函数允许填入多个类型参数，然后按std::tuple类型反序列化
 * @tparam View
 * @param v 待反序列化的数据，其包含成员函数data()和size()
 * @param offset 反序列化起始位置的偏移量。
 * @return 当用户只填入一个类型参数时，返回struct_pack::expected<T,
 * std::errc>类型，该expected类型包含反序列化结果T或std::errc类型的错误码。
 * 否则，当用户填入多个类型参数时，返回struct_pack::expected<std::tuple<T,Args...>,std::errc>类型
 */
template <typename T, detail::deserialize_view View>
[[nodiscard]] STRUCT_PACK_INLINE auto deserialize_with_offset(const View &v,
                                                              size_t &offset);

/*!
 * \ingroup struct_pack
 * 样例代码：
 *
 * ```cpp
 * person p{20, "tom"};
 * auto buffer = struct_pack::serialize(p);
 * person p2;
 * auto ec = struct_pack::deserialize_to(p2, buffer);
 * assert(ec == std::errc{});
 * assert(p == p2);
 * ```
 * @tparam T
 * @tparam Args
 * @tparam Byte
 * @param t 保存反序列化的结果。
 * @param data 指向序列化buffer的指针
 * @param size buffer的长度
 * @param args
 * 函数允许填入多个引用，然后按std::tuple<T,args...>类型解析数据并反序列化。
 * 受C++推导规则限制，args置于参数末尾。
 * @return
 * std::errc类型的错误码。另外，t作为出参保存序列化结果。当有错误发生时，t的值是未定义的。
 */
template <typename T, typename... Args, detail::struct_pack_byte Byte>
[[nodiscard]] STRUCT_PACK_INLINE std::errc deserialize_to(T &t,
                                                          const Byte *data,
                                                          size_t size,
                                                          Args... args);
/*!
 * \ingroup struct_pack
 * @tparam T
 * @tparam Args
 * @tparam View
 * @param t 保存反序列化的结果。
 * @param v 待反序列化的数据，其包含成员函数data()和size()
 * @param args
 * 函数允许填入多个引用，然后按std::tuple<T,args...>类型解析数据并反序列化。
 * 受C++推导规则限制，args置于参数末尾。
 * @return
 * std::errc类型的错误码。另外，t作为出参保存序列化结果。当有错误发生时，t的值是未定义的。
 */
template <typename T, typename... Args, detail::deserialize_view View>
[[nodiscard]] STRUCT_PACK_INLINE std::errc deserialize_to(T &t, const View &v,
                                                          Args... args);

/*!
 * \ingroup struct_pack
 * 样例代码：
 *
 * ```cpp
 *     std::string hi = "Hello struct_pack";
 * std::vector<char> ho;
 * for (int i = 0; i < 100; ++i) {
 *   struct_pack::serialize_to(ho, hi + std::to_string(i));
 * }
 * for (size_t i = 0, offset = 0; i < 100; ++i) {
 *   std::string value;
 *   auto ec = struct_pack::deserialize_to_with_offset(value, ho, offset);
 *   CHECK(ec == std::errc{});
 *   CHECK(value == hi + std::to_string(i));
 * }
 * ```
 * @tparam T
 * @tparam Args
 * @tparam Byte
 * @param t 保存反序列化的结果。
 * @param data 指向保存序列化结果的内存首地址。
 * @param size 内存的长度。
 * @param offset 反序列化起始位置的偏移量。
 * @param args
 * 函数允许填入多个引用，然后按std::tuple<T,args...>类型解析数据并反序列化。
 * 受C++推导规则限制，args置于参数末尾。
 * @return
 * std::errc类型的错误码。另外，t作为出参，保存序列化结果。offset作为出参，会被更新为反序列化对象尾部相对于data的offset。
 *                当有错误发生时，t的值是未定义的，而offset则不会被更新。
 */
template <typename T, typename... Args, detail::struct_pack_byte Byte>
[[nodiscard]] STRUCT_PACK_INLINE std::errc deserialize_to_with_offset(
    T &t, const Byte *data, size_t size, size_t &offset, Args... args);

/*!
 * \ingroup struct_pack
 * 样例代码：
 *
 * ```cpp
 * person p{20, "tom"};
 * auto buffer = serialize(p);
 *
 * auto [ec, age] = get_field<person, 0>(buffer);
 * CHECK(ec == std::errc{});
 * CHECK(age == 20);
 *
 * auto [ec, name] = get_field<person, 1>(buffer.data(), buffer.size());
 * CHECK(ec == std::errc{});
 * CHECK(name == "tom");
 * ```
 *
 * @tparam T
 * @tparam I
 * @tparam Byte
 * @param data 指向保存序列化结果的内存首地址。
 * @param size 内存的长度
 * @return
 * expected类型，包含反序列化的结果（T的第I个字段）或std::errc类型的错误码。
 */
template <typename T, size_t I, detail::struct_pack_byte Byte>
[[nodiscard]] STRUCT_PACK_INLINE auto get_field(const Byte *data, size_t size);

/*!
 * \ingroup struct_pack
 * 样例代码：
 *
 * ```cpp
 * person p{20, "tom"};
 * auto buffer = serialize(p);
 *
 * auto [ec, age] = get_field<person, 0>(buffer);
 * CHECK(ec == std::errc{});
 * CHECK(age == 20);
 *
 * auto [ec, name] = get_field<person, 1>(buffer.data(), buffer.size());
 * CHECK(ec == std::errc{});
 * CHECK(name == "tom");
 * ```
 *
 * @tparam T
 * @tparam I
 * @tparam Field
 * @tparam Byte
 * @param data 指向保存序列化结果的内存首地址。
 * @param size 内存的长度
 * @return
 * std::errc类型的错误码。另外，t作为出参，返回反序列化的结果。当有错误发生时，t的值是未定义的。
 */
template <typename T, size_t I, typename Field, detail::struct_pack_byte Byte>
[[nodiscard]] STRUCT_PACK_INLINE std::errc get_field_to(Field &t,
                                                        const Byte *data,
                                                        size_t size);

}  // namespace struct_pack
