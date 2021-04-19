#include "../rc4.h"

#include <fmt/ranges.h>
#include <unistd.h>

int main() {
  int key_n = 32;

  srand(time(nullptr));
  std::vector<uint8_t> key;
  for (int i = 0; i < key_n; ++i) {
    key.push_back(rand() % 256);
  }

  RC4 rc4(key);
  fmt::print("key: \n{}\n", key);
  std::vector<uint8_t> plain;
  for (int i = 0; i < 16; ++i) {
    plain.push_back(rand() % 256);
  }
  fmt::print("plain: \n{}\n", plain);

  auto en = rc4.DoRC4(plain);
  fmt::print("en: \n{}\n", en);

  auto de = rc4.DoRC4(en);
  fmt::print("de: \n{}\n", de);
}
