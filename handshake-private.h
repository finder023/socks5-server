/**
 * @file handshake-private.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-11
 */

#pragma once

#include "channel.h"
#include "confirm.h"
#include "event.h"
#include "handshake.h"
#include "iworker.h"
#include "protocol.h"

namespace socks5 {

class HandshakePrivate : public Event, public Handshake {
 public:
  HandshakePrivate(const int fd, IWorker* iworker)
      : Event{fd},
        iworker_{iworker},
        req_header_{reinterpret_cast<PrivateRequestHeader*>(req_buffer_)} {
    LOG("create Handshake private. fd = {}\n", fd_);
  }
  ~HandshakePrivate() { LOG("destroy Handshake private. fd = {}\n", fd_); }

  IWorker*              iworker() override { return iworker_; }
  PrivateRequestHeader* req_header() override { return req_header_; }

  const char* name() const override { return "Handshake"; }
  EventType   type() const override { return EventType::HANDSHAKE; }

  std::shared_ptr<Channel> ToChannel();
  ssize_t                  HandleReadable() override;
  ssize_t                  HandleClose() override;
  void                     ConfirmRemoteConnection() override;

 private:
  IWorker*              iworker_;
  PrivateRequestHeader* req_header_;
  uint8_t               req_buffer_[sizeof(PrivateRequestHeader) + 256];

  std::shared_ptr<Confirm> confirm_;
};

}  // namespace socks5