/**
 * @file handshake-socks5.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-11
 */

#include "handshake-socks5.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

#include "channel.h"
#include "confirm.h"
#include "log.h"
#include "protocol.h"
#include "socket-io.h"

namespace socks5 {

ssize_t HandshakeSocks5::HandleReadable() {
  LOG("shake hands readable. fd = {}\n", fd_);
  switch (status_) {
    case 0:
      if (!HandleAuth()) return -1;
      ++status_;
      break;
    case 1:
      if (!HandleResquest()) return -1;
      ++status_;
      break;
    default:
      break;
  }
  return 0;
}

std::shared_ptr<Channel> HandshakeSocks5::ToChannel() {
  auto channel = std::make_shared<Channel>(fd_, iworker_);
  fd_          = 0;
  return channel;
}

void HandshakeSocks5::ConfirmRemoteConnection() {
  LOG("confirm connection, fd = {}, req_fd = {}\n", fd_, confirm_->fd());
  ResquestReplyIpv4 reply{.ver      = 5,
                          .rep      = 0,
                          .rsv      = 0,
                          .atyp     = 1,
                          .bnd_addr = req_ip_,
                          .bnd_port = req_port_};

  if (SocketIO(fd_).Write({reinterpret_cast<uint8_t*>(&reply), sizeof(reply)}) <
      0) {
    HandleClose();
    return;
  }

  auto ev1 = this->ToChannel();
  auto ev2 = confirm_->ToChannel();

  ev1->SetPeer(ev2);
  ev2->SetPeer(ev1);

  auto c1 = std::make_shared<Channel::CacheContainer>();
  auto c2 = std::make_shared<Channel::CacheContainer>();

  ev1->SetCache(c1, c2);
  ev2->SetCache(c2, c1);

  iworker_->epoll().DelEvent(ev1->fd());
  iworker_->epoll().DelEvent(ev2->fd());

  iworker_->AddEvent(ev1);
  iworker_->AddEvent(ev2);

  iworker_->epoll().AddEvent(ev1, EPOLLIN);
  iworker_->epoll().AddEvent(ev2, EPOLLIN);
}

ssize_t HandshakeSocks5::HandleClose() {
  iworker_->AddExceptionEvent(fd_);
  if (confirm_) iworker_->AddExceptionEvent(confirm_->fd());
  return 0;
}

bool HandshakeSocks5::HandleAuth() {
  char buff[257];  // 1 + 1 + 255
  if (SocketIO(fd_).Read({reinterpret_cast<uint8_t*>(buff), sizeof(buff)}) < 0)
    return false;

  Auth* auth = reinterpret_cast<Auth*>(buff);
  if (auth->ver != 5) {
    return false;
  }

  for (uint8_t i = 0; i < auth->nmethod; ++i) {
    if (auth->method[i] != 0) continue;

    AuthReply reply = {auth->ver, auth->method[i]};
    if (SocketIO(fd_).Write(
            {reinterpret_cast<uint8_t*>(&reply), sizeof(reply)}) < 0)
      return false;
    return true;
  }

  return false;
}

bool HandshakeSocks5::ParseRemoteAddr() {
  char buff[4096];
  if (SocketIO(fd_).Read({reinterpret_cast<uint8_t*>(buff), sizeof(buff)}) < 0)
    return false;

  Request* req = reinterpret_cast<Request*>(buff);
  if (req->ver != 5) {
    return false;
  }

  if (req->cmd != 1) {  // connect
    return false;
  }

  if (req->address_type == 1) {  // ipv4 addr
    if (iworker_->deploy() == Deploy::SERVER) {
      memcpy(&req_ip_, req->dest_addr, sizeof(req_ip_));
      memcpy(&req_port_, req->dest_addr + 4, sizeof(req_port_));
    } else if (iworker_->deploy() == Deploy::LOCAL) {
      memcpy(&req_header_->address, req->dest_addr, 4);
      memcpy(&req_header_->port, req->dest_addr + 4, 2);
      req_header_->type      = 1;
      req_header_->fd        = fd_;
      req_header_->addr_len  = 4;
      req_header_->timestamp = time(nullptr);
    }
  } else if (req->address_type == 3) {
    if (iworker_->deploy() == Deploy::SERVER) {
      req_ip_ = QueryDNS(reinterpret_cast<char*>(req->dest_addr) + 1,
                         req->dest_addr[0]);
      if (req_ip_ < 0) return false;
      memcpy(&req_port_, req->dest_addr + 1 + req->dest_addr[0],
             sizeof(req_port_));
      LOG("target: {}:{}\n", inet_ntoa(*reinterpret_cast<in_addr*>(&req_ip_)),
          ntohs(req_port_));
    } else if (iworker_->deploy() == Deploy::LOCAL) {
      // copy domain
      memcpy(req_header_->address, req->dest_addr + 1, req->dest_addr[0]);
      // copy port
      memcpy(&req_header_->port, req->dest_addr + 1 + req->dest_addr[0], 2);
      req_header_->type      = 2;
      req_header_->fd        = fd_;
      req_header_->addr_len  = req->dest_addr[0];
      req_header_->timestamp = time(nullptr);
    }

  } else {
    return false;
  }
  return true;
}

bool HandshakeSocks5::HandleResquest() {
  if (!ParseRemoteAddr()) return false;

  ResquestReplyIpv4 reply{.ver      = 5,
                          .rep      = 0,
                          .rsv      = 0,
                          .atyp     = 1,
                          .bnd_addr = req_ip_,
                          .bnd_port = req_port_};

  sockaddr_in sin;
  if (iworker_->deploy() == Deploy::SERVER) {
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = req_ip_;
    sin.sin_port        = req_port_;
  } else if (iworker_->deploy() == Deploy::LOCAL) {
    sin = iworker_->static_addr();
  }

  int fd;
  if ((fd = TcpConnect(sin)) <= 0) {
    reply.rep = 1;
    if (SocketIO(fd_).Write(
            {reinterpret_cast<uint8_t*>(&reply), sizeof(reply)}) < 0)
      return false;
  }

  confirm_ = std::make_shared<Confirm>(fd, this);
  iworker_->epoll().DelEvent(fd_);  // wait for connction confirm.

  iworker_->AddEvent(confirm_);
  iworker_->epoll().AddEvent(confirm_, EPOLLOUT);

  return true;
}

}  // namespace socks5