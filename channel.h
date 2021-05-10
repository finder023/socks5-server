/**
 * @file channel.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-11
 */
#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>

#include <memory>

#include "buffer.h"
#include "event.h"
#include "iworker.h"
#include "log.h"
#include "socket-io.h"

namespace socks5 {

class Channel : public Event {
 public:
  Channel(const int fd, IWorker* worker) : Event{fd}, iworker_{worker} {
    LOG("create new Channel. fd = {}\n", fd_);
  }

  ~Channel() { LOG("Channel destroied. fd = {}\n", fd_); }

  void SetPeer(const std::shared_ptr<Event>& ev) { peer_ = ev; }
  const std::shared_ptr<Event> peer() const { return peer_.lock(); }

  ssize_t HandleLoop() override {
    if (!peer_.lock()) return -1;

    if (cache_.size - cache_.seek > 0) {
      ssize_t n = SocketIO::WriteSocket(
          peer_.lock()->fd(),
          {cache_.memory + cache_.seek, cache_.size - cache_.seek});
      if (n < 0) {
        return -1;
      }
      cache_.seek += n;
    }

    return 0;
  }

  ssize_t HandleReadable() override {
    if (cache_.seek != 0) cache_.Shift();

    ssize_t n;
    if (cache_.capacity - cache_.size > 0) {
      n = SocketIO::ReadSocket(
          fd_, {cache_.memory + cache_.size, cache_.capacity - cache_.size});
      if (n < 0) {
        return -1;
      }
      cache_.size += n;
    }

    return 0;
  }

  ssize_t HandleClose() override {
    iworker_->AddExceptionEvent(fd_);
    if (peer_.lock()) {
      iworker_->AddExceptionEvent(peer_.lock()->fd());
    }
    return 0;
  }

 private:
  std::weak_ptr<Event>                peer_;
  IWorker*                            iworker_;
  std::shared_ptr<Container<0x4000>>  recv_cache_;
  std::shared_ptr<Container<0x10000>> send_cache_;

  Container<0x10000> cache_;
};
}  // namespace socks5
