/**
 * @file encryptor.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-12
 */

#pragma once

#include <netinet/in.h>

#include <memory>
#include <vector>

#include "buffer.h"
#include "log.h"
#include "protocol.h"
#include "rc4.h"

namespace socks5 {

class EncryptorBase {
 public:
  EncryptorBase()
      : static_key_{1,  54,  67,  32,  54,  68,  253, 98, 63, 97,
                    27, 187, 194, 204, 149, 168, 238, 4,  28, 98,
                    46, 24,  65,  94,  28,  37,  83,  19},
        key_{static_key_.begin(), static_key_.end()} {
    rc4_ = std::make_shared<RC4>(key_);
  }

  EncryptorBase(const PrivateRequestHeader* header)
      : static_key_{1,  54,  67,  32,  54,  68,  253, 98, 63, 97,
                    27, 187, 194, 204, 149, 168, 238, 4,  28, 98,
                    46, 24,  65,  94,  28,  37,  83,  19} {
    bzero(key_buffer, sizeof(key_buffer));
    memcpy(key_buffer, header, sizeof(PrivateRequestHeader) + header->addr_len);
    key_ = GenKey(reinterpret_cast<PrivateRequestHeader*>(key_buffer));
    rc4_ = std::unique_ptr<RC4>(new RC4(key_));
  }

  virtual void Process(const Buffer& buffer) = 0;

  void NaiveEncrypt(const Buffer& buffer) {
    for (uint8_t *p = buffer.memory, *ed = buffer.memory + buffer.capacity;
         p < ed; ++p) {
      *p = ~(*p);
      *p ^= MASK;
      *p = Move(*p, 3);
    }
  }

  void NaiveDecrypt(const Buffer& buffer) {
    for (uint8_t *p = buffer.memory, *ed = buffer.memory + buffer.capacity;
         p < ed; ++p) {
      *p = Move(*p, 5);
      *p ^= MASK;
      *p = ~(*p);
    }
  }

  std::vector<uint8_t> GenKey(PrivateRequestHeader* header) {
    Buffer buff{reinterpret_cast<uint8_t*>(header),
                sizeof(PrivateRequestHeader) + header->addr_len};

    NaiveEncrypt(buff);

    uint64_t tmp[buff.capacity];
    for (int i = 0; i < buff.capacity; ++i) {
      tmp[i] = htonl(header->timestamp) * (*(buff.memory + i));
    }

    return {reinterpret_cast<uint8_t*>(tmp),
            reinterpret_cast<uint8_t*>(tmp) + sizeof(tmp)};
  }

 protected:
  uint8_t Move(const uint8_t b, uint32_t len) {
    return (b << len) | (b >> (8 - len));
  }

 protected:
  static constexpr uint8_t MASK = 0b11101011;
  std::array<uint8_t, 28>  static_key_;
  std::vector<uint8_t>     key_;
  std::shared_ptr<RC4>     rc4_;
  uint8_t                  key_buffer[sizeof(PrivateRequestHeader) + 256];
};

class Encryptor : public EncryptorBase {
 public:
  Encryptor() : EncryptorBase{} {}
  Encryptor(const PrivateRequestHeader* header) : EncryptorBase{header} {}

  void Process(const Buffer& buffer) override {
    NaiveEncrypt(buffer);
    rc4_->DoRC4(buffer.memory, buffer.capacity);
  }
};

class Decryptor : public EncryptorBase {
 public:
  Decryptor() : EncryptorBase{} {}
  Decryptor(const PrivateRequestHeader* header) : EncryptorBase{header} {}

  void Process(const Buffer& buffer) override {
    rc4_->DoRC4(buffer.memory, buffer.capacity);
    NaiveDecrypt(buffer);
  }
};

}  // namespace socks5
