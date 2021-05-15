/**
 * @file handshake-ss.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-13
 */
#pragma once

#include "channel.h"
#include "confirm.h"
#include "event.h"
#include "handshake.h"
#include "iworker.h"
#include "protocol.h"

namespace socks5 {

class HandshakeSS : public Event, public Handshake {
 public:
  HandshakeSS(const int fd, IWorker* iworker)
      : Event{fd},
        iworker_{iworker},
        req_header_{reinterpret_cast<PrivateRequestHeader*>(req_buffer_)} {
    LOG("create Handshake ss. fd = %d\n", fd_);
  }
  ~HandshakeSS() { LOG("destroy Handshake ss. fd = %d\n", fd_); }

  IWorker*              iworker() override { return iworker_; }
  PrivateRequestHeader* req_header() override { return req_header_; }

  const char* name() const override { return "Handshake"; }
  EventType   type() const override { return EventType::HANDSHAKE; }

  std::shared_ptr<Channel> ToChannel();
  ssize_t                  HandleReadable() override;
  ssize_t                  HandleClose() override;
  void                     ConfirmRemoteConnection() override;

 private:
  IWorker*                 iworker_;
  std::shared_ptr<Confirm> confirm_;
  uint32_t                 req_ip_;
  uint16_t                 req_port_;

  char                  domain_[256] = {0};
  PrivateRequestHeader* req_header_;
  uint8_t               req_buffer_[sizeof(PrivateRequestHeader) + 256];
};

}  // namespace socks5
