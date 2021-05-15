/**
 * @file confirm.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-11
 */
#pragma once

#include "channel.h"
#include "event.h"
#include "handshake.h"

namespace socks5 {

class Confirm : public Event {
 public:
  Confirm(const int fd, Handshake* hand_shake)
      : Event{fd}, hand_shake_{hand_shake} {
    LOG("Confirm connection created. fd = %d\n", fd_);
  }
  ~Confirm() { LOG("Confirm connection destroied. fd = %d\n", fd_); }

  const char* name() const override { return "Confirm"; }
  EventType   type() const override { return EventType::CONFIRM; }

  std::shared_ptr<Channel> ToChannel();
  ssize_t                  HandleWritable() override;
  ssize_t                  HandleClose() override;

  Handshake* hand_shake_;
};
}  // namespace socks5