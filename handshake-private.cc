/**
 * @file handshake-private.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-11
 */

#include "handshake-private.h"

namespace socks5 {

ssize_t HandshakePrivate::HandleReadable() {
  uint32_t    ip  = 0;
  int         fd  = 0;
  sockaddr_in sin = {0};

  Buffer req_buffer{req_buffer_, sizeof(PrivateRequestHeader)};
  if (SocketIO(fd_).Read(req_buffer) < 0) return -1;
  Buffer addr_payload{req_buffer_ + sizeof(PrivateRequestHeader),
                      req_header_->addr_len};
  if (SocketIO(fd_).Read(addr_payload) < 0) return -1;

  Buffer req_payload{req_buffer_, req_buffer.capacity + addr_payload.capacity};
  if (iworker_->encrypt()) {
    Decryptor{}.NaiveDecrypt(req_payload);
  }

  if (req_header_->type == 1) {
    ip = *reinterpret_cast<uint32_t*>(req_header_->address);
  } else if (req_header_->type == 2) {
    // query domain
    ip = QueryDNS(req_header_->address, req_header_->addr_len);
    if (ip <= 0) return -1;
    LOG("read %s target addr: %s:%d\n", req_header_->address,
        inet_ntoa(in_addr{ip}), ntohs(uint16_t(req_header_->port)));
  }

  sin.sin_family      = AF_INET;
  sin.sin_addr.s_addr = ip;
  sin.sin_port        = req_header_->port;

  // remote connction 不再关注可读，后续读写交给channel
  iworker_->epoll().DelEvent(fd_);

  fd = TcpConnect(sin);
  if (fd <= 0) return -1;
  // wait for confirm connection
  confirm_ = std::make_shared<Confirm>(fd, this);
  iworker_->AddEvent(confirm_);
  iworker_->epoll().AddEvent(confirm_, EPOLLOUT);
  return true;
}

std::shared_ptr<Channel> HandshakePrivate::ToChannel() {
  auto channel = std::make_shared<Channel>(fd_, iworker_);
  fd_          = 0;
  return channel;
}

ssize_t HandshakePrivate::HandleClose() {
  iworker_->AddExceptionEvent(fd_);
  if (confirm_) {
    iworker_->AddExceptionEvent(confirm_->fd());
  }
  return 0;
}

void HandshakePrivate::ConfirmRemoteConnection() {
  LOG("confirm connection. fd = %d, req_fd = %d\n", fd_, confirm_->fd());
  auto ev1 = this->ToChannel();
  auto ev2 = confirm_->ToChannel();

  if (iworker_->encrypt()) {
    ev1->SetEncryptor(std::make_shared<Decryptor>(req_header_));
    ev2->SetEncryptor(std::make_shared<Encryptor>(req_header_));
  }

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