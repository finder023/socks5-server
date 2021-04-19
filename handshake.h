/**
 * @file worker.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#pragma once

#include "channel.h"
#include "event.h"
#include "iworker.h"

namespace socks5 {

// class Handshake {
//  public:
//   Handshake(const int fd) : fd_{fd} {}
//   int Build();  // return fd
//
//  private:
//   bool HandleAuth();
//   bool HandleResquest();
//
//   bool ParseRemoteAddr();
//   bool ConnectRemote();
//
//  private:
//   int      fd_;
//   int      req_fd_;
//   uint32_t req_ip_;
//   uint16_t req_port_;
// };

class Handshake : public Event {
 public:
  Handshake(const int fd, IWorker* worker)
      : Event{fd}, status_{0}, iworker_{worker}, req_fd_{0} {}
  int Build();  // return fd

 private:
  bool HandleAuth();
  bool HandleResquest();
  bool HandleData();

  ssize_t HandleReadable() override;
  ssize_t HandleClose() override;

  bool ParseRemoteAddr();
  bool ConnectRemote();

 private:
  int      status_;
  IWorker* iworker_;
  int      req_fd_;
  uint32_t req_ip_;
  uint16_t req_port_;
};

}  // namespace socks5
