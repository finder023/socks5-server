/**
 * @file worker.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#pragma once

#include "channel.h"
#include "iworker.h"

namespace socks5 {

class Handshake {
 public:
  Handshake(const int fd) : fd_{fd} {}
  int Build();  // return fd

 private:
  bool HandleAuth();
  bool HandleResquest();

  bool ParseRemoteAddr();
  bool ConnectRemote();

 private:
  int      fd_;
  int      req_fd_;
  uint32_t req_ip_;
  uint16_t req_port_;
};
}  // namespace socks5
