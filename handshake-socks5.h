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
        req_header_{reinterpret_cast<PrivateRequestHeader*>(req_buffer_)} {
    LOG("create Handshake socks5. fd = {}\n", fd_);
  }
  ~HandshakeSocks5() { LOG("destroy Handshake socks5, fd = {}\n", fd_); }

  const char* name() const override { return "Handshake"; }
  EventType   type() const override { return EventType::HANDSHAKE; }

  IWorker*                 iworker() override { return iworker_; }
  std::shared_ptr<Channel> ToChannel();
  void                     ConfirmRemoteConnection() override;
  PrivateRequestHeader*    req_header() override { return req_header_; }

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
  uint32_t req_ip_;
  uint16_t req_port_;

  PrivateRequestHeader*    req_header_;
  uint8_t                  req_buffer_[sizeof(PrivateRequestHeader) + 256];
  std::shared_ptr<Confirm> confirm_;
};

}  // namespace socks5
