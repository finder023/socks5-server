/**
 * @file listener.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-11
 */
#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

#include "event.h"
#include "iworker.h"

namespace socks5 {

class Listener : public Event {
 public:
  Listener(const uint16_t port, IWorker* worker)
      : Event{0}, port_{port}, iworker_{worker} {}

  std::shared_ptr<Event> StartListener();
  ssize_t                HandleReadable() override;

 private:
  uint16_t port_;
  IWorker* iworker_;
};

}  // namespace socks5
