/**
 * @file worker.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#pragma once

#include "channel.h"
#include "event.h"
#include "iworker.h"

namespace socks5 {

class RemoteConn;

class Handshake : public Event {
 public:
  Handshake(const int fd, IWorker* worker)
      : Event{fd}, status_{0}, iworker_{worker}, req_fd_{0} {}

  IWorker*                 iworker() { return iworker_; }
  std::shared_ptr<Channel> ToChannel();
  void                     ConfirmRemoteConnection();

 private:
  bool HandleAuth();
  bool HandleResquest();
  bool HandleData();

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

  std::shared_ptr<RemoteConn> remote_conn_;
};

class RemoteConn : public Event {
 public:
  RemoteConn(const int fd, Handshake* hand_shake)
      : Event{fd}, hand_shake_{hand_shake} {}

  std::shared_ptr<Channel> ToChannel() {
    auto channel = std::make_shared<Channel>(fd_, hand_shake_->iworker());
    fd_          = 0;
    return channel;
  }

  ssize_t HandleWritable() override {
    uint32_t  val;
    socklen_t len = sizeof(val);
    getsockopt(fd_, SOL_SOCKET, SO_ERROR, &val, &len);
    if (val == 0) {
      hand_shake_->ConfirmRemoteConnection();
      return 0;
    }
    return val;
  }

  ssize_t HandleClose() override {
    hand_shake_->iworker()->AddExceptionEvent(fd_);
    return 0;
  }

  Handshake* hand_shake_;
};

}  // namespace socks5
