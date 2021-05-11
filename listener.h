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
  Listener(const sockaddr_in& listen, IWorker* worker)
      : Event{0}, listen_{listen}, iworker_{worker} {}

  bool    StartListener();
  ssize_t HandleReadable() override;

 private:
  sockaddr_in listen_;
  IWorker*    iworker_;
};

}  // namespace socks5
