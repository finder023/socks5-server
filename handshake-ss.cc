/**
 * @file handshake-ss.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-13
 */

#include "handshake-ss.h"

namespace socks5 {

ssize_t HandshakeSS::HandleReadable() {
  /*
   * Shadowsocks TCP Relay Header:
   *
   *    +------+----------+----------+
   *    | ATYP | DST.ADDR | DST.PORT |
   *    +------+----------+----------+
   *    |  1   | Variable |    2     |
   *    +------+----------+----------+
   *
   */

  uint8_t buff[256] = {0};
  Buffer  tmp{buff, 1};
  uint8_t name_len;
  SocketIO(fd_).Read(tmp);
  uint8_t type = *buff;
  if (type == 1) {
    tmp = {buff, 6};
    SocketIO(fd_).Read(tmp);
    req_ip_   = *reinterpret_cast<uint32_t*>(buff);
    req_port_ = *reinterpret_cast<uint16_t*>(buff + 4);
  } else if (type == 3) {
    tmp = {buff, 1};
    SocketIO(fd_).Read(tmp);
    name_len = *buff;
    tmp      = {buff, static_cast<uint64_t>(name_len) + 2};
    SocketIO(fd_).Read(tmp);
    memcpy(domain_, buff, name_len);
    LOG("{} read domain: {}\n", fd_, domain_);
    req_port_ = *reinterpret_cast<uint16_t*>(buff + name_len);
  } else {
    // not support
    return -1;
  }

  iworker_->epoll().DelEvent(fd_);
  sockaddr_in sin = {0};
  sin.sin_family  = AF_INET;
  sin.sin_port    = req_port_;
  int fd          = 0;

  if (iworker_->deploy() == Deploy::SERVER) {
    if (type == 1) {
      sin.sin_addr.s_addr = req_ip_;
    } else if (type == 3) {
      req_ip_ = QueryDNS(domain_, name_len);
      if (req_ip_ < 0) return -1;

      sin.sin_addr.s_addr = req_ip_;
      LOG("domain target: {}:{}\n", domain_,
          inet_ntoa(*reinterpret_cast<in_addr*>(&req_ip_)), ntohs(req_port_));
    }

    if ((fd = TcpConnect(sin)) < 0) return -1;

  } else if (iworker_->deploy() == Deploy::LOCAL) {
    // local
    if (type == 1) {
      req_header_->type     = 1;
      req_header_->addr_len = 4;
      memcpy(req_header_->address, &req_ip_, sizeof(req_ip_));
    } else if (type == 3) {
      req_header_->type     = 2;
      req_header_->addr_len = name_len;
      memcpy(req_header_->address, domain_, name_len);
    }
    req_header_->port      = req_port_;
    req_header_->fd        = fd_;
    req_header_->timestamp = time(nullptr);

    sin = iworker_->static_addr();
    if ((fd = TcpConnect(sin)) < 0) return -1;
  }

  confirm_ = std::make_shared<Confirm>(fd, this);
  iworker_->AddEvent(confirm_);
  iworker_->epoll().AddEvent(confirm_, EPOLLOUT);
  return 0;
}

std::shared_ptr<Channel> HandshakeSS::ToChannel() {
  auto channel = std::make_shared<Channel>(fd_, iworker_);
  fd_          = 0;
  return channel;
}

ssize_t HandshakeSS::HandleClose() {
  iworker_->AddExceptionEvent(fd_);
  if (confirm_) {
    iworker_->AddExceptionEvent(confirm_->fd());
  }
  return 0;
}

void HandshakeSS::ConfirmRemoteConnection() {
  LOG("confirm connection. fd = {}, req_fd = {}\n", fd_, confirm_->fd());
  auto ev1 = this->ToChannel();
  auto ev2 = confirm_->ToChannel();

  auto c1 = std::make_shared<Channel::CacheContainer>();
  auto c2 = std::make_shared<Channel::CacheContainer>();

  ev1->SetCache(c1, c2);
  ev2->SetCache(c2, c1);

  ev1->SetPeer(ev2);
  ev2->SetPeer(ev1);

  if (iworker_->encrypt()) {
    ev1->SetEncryptor(std::make_shared<Encryptor>(req_header_));
    ev2->SetEncryptor(std::make_shared<Decryptor>(req_header_));
  }

  iworker_->epoll().DelEvent(ev1->fd());
  iworker_->epoll().DelEvent(ev2->fd());

  iworker_->AddEvent(ev1);
  iworker_->AddEvent(ev2);

  iworker_->epoll().AddEvent(ev1, EPOLLIN);
  iworker_->epoll().AddEvent(ev2, EPOLLIN);
}

}  // namespace socks5
