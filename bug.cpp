#include "protopuf/message.h"
using namespace pp;
int main() {
  using Book = message<
      string_field<"name", 1>
      >;
  using Student = message<
      uint32_field<"id", 1>,
      string_field<"name", 3>,
      message_field<"books", 5, Book, repeated>
      >;
  using Class = message<
      message_field<"students", 1, Student, repeated>
      >;

  std::array<std::byte, 64> buffer{};
  Class c;
  message_coder<Class>::encode(c, buffer);
  return 0;
}