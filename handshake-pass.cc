/**
 * @file handshake-pass.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-13
 */

#include "handshake-pass.h"

namespace socks5 {

ssize_t HandshakePass::HandleReadable() {
  int         fd  = 0;
  sockaddr_in sin = iworker_->static_addr();

  iworker_->epoll().DelEvent(fd_);

  if ((fd = TcpConnect(sin)) < 0) return -1;

  confirm_ = std::make_shared<Confirm>(fd, this);
  iworker_->AddEvent(confirm_);
  iworker_->epoll().AddEvent(confirm_, EPOLLOUT);

  return 0;
}

std::shared_ptr<Channel> HandshakePass::ToChannel() {
  auto channel = std::make_shared<Channel>(fd_, iworker_);
  fd_          = 0;
  return channel;
}

ssize_t HandshakePass::HandleClose() {
  iworker_->AddExceptionEvent(fd_);
  if (confirm_) {
    iworker_->AddExceptionEvent(confirm_->fd());
  }
  return 0;
}

void HandshakePass::ConfirmRemoteConnection() {
  LOG("confirm connection. fd = {}, req_fd = {}\n", fd_, confirm_->fd());
  auto ev1 = this->ToChannel();
  auto ev2 = confirm_->ToChannel();

  auto c1 = std::make_shared<Channel::CacheContainer>();
  auto c2 = std::make_shared<Channel::CacheContainer>();

  ev1->SetCache(c1, c2);
  ev2->SetCache(c2, c1);

  ev1->SetPeer(ev2);
  ev2->SetPeer(ev1);

  iworker_->epoll().DelEvent(ev1->fd());
  iworker_->epoll().DelEvent(ev2->fd());

  iworker_->AddEvent(ev1);
  iworker_->AddEvent(ev2);

  iworker_->epoll().AddEvent(ev1, EPOLLIN);
  iworker_->epoll().AddEvent(ev2, EPOLLIN);
}

}  // namespace socks5
