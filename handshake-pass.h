/**
 * @file handshake-pass.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief pass through
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

class HandshakePass : public Event, public Handshake {
 public:
  HandshakePass(const int fd, IWorker* iworker) : Event{fd}, iworker_{iworker} {
    LOG("create Handshake pass. fd = {}\n", fd_);
  }
  ~HandshakePass() { LOG("destroy Handshake pass. fd = {}\n", fd_); }

  IWorker*              iworker() override { return iworker_; }
  PrivateRequestHeader* req_header() override { return nullptr; }

  const char* name() const override { return "Handshake"; }
  EventType   type() const override { return EventType::HANDSHAKE; }

  std::shared_ptr<Channel> ToChannel();
  ssize_t                  HandleReadable() override;
  ssize_t                  HandleClose() override;
  void                     ConfirmRemoteConnection() override;

 private:
  IWorker*                 iworker_;
  std::shared_ptr<Confirm> confirm_;
};

}  // namespace socks5
