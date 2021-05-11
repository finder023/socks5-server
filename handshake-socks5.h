/**
 * @file worker.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#pragma once

#include "channel.h"
#include "event.h"
#include "handshake.h"
#include "iworker.h"
#include "protocol.h"

namespace socks5 {

class Confirm;

class HandshakeSocks5 : public Event, public Handshake {
 public:
  HandshakeSocks5(const int fd, IWorker* worker)
      : Event{fd},
        status_{0},
        iworker_{worker},
        req_fd_{0},
        req_header_{reinterpret_cast<PrivateRquestHeader*>(req_buffer_)} {
    LOG("create Handshake socks5. fd = {}\n", fd_);
  }
  ~HandshakeSocks5() { LOG("destroy Handshake socks5, fd = {}\n", fd_); }

  IWorker*                 iworker() override { return iworker_; }
  std::shared_ptr<Channel> ToChannel();
  void                     ConfirmRemoteConnection() override;

 private:
  bool HandleAuth();
  bool HandleResquest();

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

  PrivateRquestHeader*     req_header_;
  uint8_t                  req_buffer_[sizeof(PrivateRquestHeader) + 256];
  std::shared_ptr<Confirm> confirm_;
};

}  // namespace socks5
