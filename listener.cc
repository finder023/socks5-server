/**
 * @file listener.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-11
 */

#include "listener.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "channel.h"
#include "handshake-pass.h"
#include "handshake-private.h"
#include "handshake-socks5.h"
#include "log.h"

namespace socks5 {

bool Listener::StartListener() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd <= 0) {
    LOG(stderr, "call socket failed. err = {}\n", strerror(errno));
    return false;
  }

  if (bind(fd, reinterpret_cast<const sockaddr*>(&listen_), sizeof(listen_)) !=
      0) {
    LOG(stderr, "call bind failed. err = {}\n", strerror(errno));
    close(fd);
    return false;
  }

  if (listen(fd, 1024) != 0) {
    LOG(stderr, "call listen failed. err = {}\n", strerror(errno));
    close(fd);
    return false;
  }

  fd_ = fd;
  return true;
}

ssize_t Listener::HandleReadable() {
  sockaddr_in sin;
  socklen_t   len = sizeof(sin);

  int fd = accept4(fd_, reinterpret_cast<sockaddr*>(&sin), &len, SOCK_NONBLOCK);
  if (fd <= 0) {
    LOG(stderr, "failed to call accept. err = {}\n", strerror(errno));
    return -1;
  }

  LOG("accept from {}:{}. fd = {}\n", inet_ntoa(sin.sin_addr),
      ntohs(sin.sin_port), fd);

  std::shared_ptr<Event> hand_shake;
  if (iworker_->deploy() == Deploy::SERVER) {
    // server only support private protocol
    hand_shake = std::make_shared<HandshakePrivate>(fd, iworker_);
  }

  // local
  if (iworker_->deploy() == Deploy::LOCAL and
      iworker_->protocol() == Protocol::SOCKS5) {
    // local socks5 support
    hand_shake = std::make_shared<HandshakeSocks5>(fd, iworker_);
  }

  if (iworker_->deploy() == Deploy::LOCAL and
      iworker_->protocol() == Protocol::PASS) {
    // local pass through
    hand_shake = std::make_shared<HandshakePass>(fd, iworker_);
  }

  iworker_->AddEvent(hand_shake);
  iworker_->epoll().AddEvent(hand_shake, EPOLLIN);

  return 0;
}

}  // namespace socks5
