/**
 * @file worker.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */
#pragma once

#include <array>
#include <string>
#include <vector>

class RC4 {
 public:
  RC4(const std::vector<uint8_t>& key) : key_{key}, i_{0}, j_{0} { KSA(); }

  std::vector<uint8_t> DoRC4(const std::vector<uint8_t>& p) {
    return PRGA(p.data(), p.size());
  }

  auto& s_box() const { return s_box_; }

 private:
  static constexpr uint32_t N = 256;

  void KSA() {
    for (uint32_t i = 0; i < N; ++i) {
      s_box_[i] = i;
    }

    uint8_t j = 0;
    for (uint32_t i = 0; i < N; ++i) {
      j = (j + s_box_[i] + key_[i % key_.size()]) % N;
      Swap(i, j);
    }
  }

  std::vector<uint8_t>& PRGA(const uint8_t* plaintext, const uint32_t len) {
    int                         i = i_, j = j_;
    static std::vector<uint8_t> result;
    result.clear();
    for (size_t n = 0; n < len; ++n) {
      i = (i + 1) % N;
      j = (j + s_box_[i]) % N;
      Swap(i, j);
      uint8_t r = s_box_[(s_box_[i] + s_box_[j]) % N];
      result.push_back(r ^ plaintext[n]);
    }
    i_ = i;
    j_ = j;

    return result;
  }

  void Swap(uint32_t i, uint32_t j) {
    uint8_t t = s_box_[i];
    s_box_[i] = s_box_[j];
    s_box_[j] = t;
  }

 private:
  std::array<uint8_t, N> s_box_;
  std::vector<uint8_t>   key_;
  int                    i_;
  int                    j_;
};
