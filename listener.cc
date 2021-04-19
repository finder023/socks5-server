/**
 * @file listener.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-11
 */

#include "listener.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "channel.h"
#include "handshake.h"

namespace socks5 {

bool Listener::StartListener() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd <= 0) {
    fmt::print(stderr, "call socket failed. err = {}\n", strerror(errno));
    return false;
  }

  sockaddr_in sin{0};
  sin.sin_family      = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port        = htons(port_);

  if (bind(fd, reinterpret_cast<const sockaddr*>(&sin), sizeof(sin)) != 0) {
    fmt::print(stderr, "call bind failed. err = {}\n", strerror(errno));
    close(fd);
    return false;
  }

  if (listen(fd, 1024) != 0) {
    fmt::print(stderr, "call listen failed. err = {}\n", strerror(errno));
    close(fd);
    return false;
  }

  fd_ = fd;
  return true;
}

ssize_t Listener::HandleReadable() {
  sockaddr_in sin;
  socklen_t   len = sizeof(sin);

  int fd = accept(fd_, reinterpret_cast<sockaddr*>(&sin), &len);
  if (fd <= 0) {
    fmt::print(stderr, "failed to call accept. err = {}\n", strerror(errno));
    return -1;
  }

  fmt::print("accept from {}:{}. fd = {}\n", inet_ntoa(sin.sin_addr),
             ntohs(sin.sin_port), fd);

  auto hand_shake     = std::make_shared<Handshake>(fd, iworker_);
  iworker_->event(fd) = hand_shake;
  iworker_->epoll().AddEvent(hand_shake, EPOLLIN);
  //  auto ev = std::make_shared<Channel>(fd, iworker_);
  //  // handshake
  //  auto remote_fd = Handshake(ev->fd()).Build();
  //  if (remote_fd <= 0) return -1;
  //  auto peer_ev = std::make_shared<Channel>(remote_fd, iworker_);
  //
  //  // set peer
  //  ev->SetPeer(peer_ev);
  //  peer_ev->SetPeer(ev);
  //
  //  iworker_->event(fd)            = std::dynamic_pointer_cast<Event>(ev);
  //  iworker_->event(peer_ev->fd()) =
  //  std::dynamic_pointer_cast<Event>(peer_ev);
  //
  //  iworker_->epoll().AddEvent(std::dynamic_pointer_cast<Event>(ev), EPOLLIN);
  //  iworker_->epoll().AddEvent(std::dynamic_pointer_cast<Event>(peer_ev),
  //                             EPOLLIN);

  return 0;
}

}  // namespace socks5
