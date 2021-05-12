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
#include "encryptor.h"
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

  using CacheContainer = Container<0x20000>;

  const char* name() const override { return "Channel"; }
  EventType   type() const override { return EventType::CHANNEL; }

  void SetPeer(const std::shared_ptr<Channel>& ev) { peer_ = ev; }
  void SetCache(const std::shared_ptr<CacheContainer>& recv,
                const std::shared_ptr<CacheContainer>& send) {
    recv_cache_ = recv;
    send_cache_ = send;
  }
  void SetEncryptor(const std::shared_ptr<EncryptorBase>& en) {
    encryptor_ = en;
  }

  const std::shared_ptr<Channel> peer() const { return peer_.lock(); }

  ssize_t HandleLoop() override {
    if (send_cache_->size <= send_cache_->seek) return 0;
    ssize_t n = SocketIO(fd_).Write({send_cache_->memory + send_cache_->seek,
                                     send_cache_->size - send_cache_->seek});
    if (n < 0) return -1;
    send_cache_->seek += n;

    if (send_cache_->size > send_cache_->seek) return 1;
    return 0;
  }

  ssize_t HandleWritable() override {
    iworker_->epoll().ModEvent(fd_, EPOLLIN);  // do not care epollout
    if (!peer_.lock()) return -1;
    iworker_->epoll().ModEvent(peer_.lock()->fd(),
                               EPOLLIN);  // register peer in event
    if (send_cache_->size > send_cache_->seek) iworker_->AddLoopEvent(fd_);
    return 0;
  }

  ssize_t HandleReadable() override {
    if (recv_cache_->seek != 0) recv_cache_->Shift();
    if (recv_cache_->capacity <= recv_cache_->size) return 0;

    Buffer  buff{recv_cache_->memory + recv_cache_->size,
                recv_cache_->capacity - recv_cache_->size};
    ssize_t n = SocketIO(fd_).Read(buff);
    if (n < 0) return -1;
    recv_cache_->size += n;
    buff.capacity = n;

    if (iworker_->encrypt()) encryptor_->Process(buff);

    if (!peer_.lock()) return -1;
    if (recv_cache_->size > recv_cache_->seek) {
      iworker_->AddLoopEvent(peer_.lock()->fd());
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
  std::weak_ptr<Channel>          peer_;
  IWorker*                        iworker_;
  std::shared_ptr<CacheContainer> recv_cache_;
  std::shared_ptr<CacheContainer> send_cache_;
  std::shared_ptr<EncryptorBase>  encryptor_;
};
}  // namespace socks5
