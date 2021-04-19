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
  fmt::print("key: {}\n", key);
  std::vector<uint8_t> plain;
  for (int i = 0; i < 15; ++i) {
    plain.push_back(rand() % 256);
  }
  fmt::print("plain: {}\n", plain);

  auto en = rc4.DoRC4(plain);
  fmt::print("en: {}\n", en);

  auto front_part = std::vector<uint8_t>{en.begin(), en.begin() + 10};
  auto left_part  = std::vector<uint8_t>(en.begin() + 10, en.end());
  RC4  rc4de(key);
  auto de = rc4de.DoRC4(front_part);
  fmt::print("front: {}\n", de);
  auto de1 = rc4de.DoRC4(left_part);
  fmt::print("left: {}\n", de1);
}
