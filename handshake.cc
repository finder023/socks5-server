/**
 * @file worker.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#include "handshake.h"

#include <arpa/inet.h>
#include <fmt/format.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

#include "channel.h"
#include "protocol.h"
#include "socket-io.h"

namespace socks5 {

ssize_t Handshake::HandleReadable() {
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

std::shared_ptr<Channel> Handshake::ToChannel() {
  auto channel = std::make_shared<Channel>(fd_, iworker_);
  fd_          = 0;
  return channel;
}

void Handshake::ConfirmRemoteConnection() {
  fmt::print("confirm connection\n");
  ResquestReplyIpv4 reply{.ver      = 5,
                          .rep      = 0,
                          .rsv      = 0,
                          .atyp     = 1,
                          .bnd_addr = req_ip_,
                          .bnd_port = req_port_};

  if (SocketIO::WriteSocket(
          fd_, {reinterpret_cast<uint8_t*>(&reply), sizeof(reply)}) < 0) {
    HandleClose();
    return;
  }

  auto ev1 = this->ToChannel();
  auto ev2 = remote_conn_->ToChannel();

  ev1->SetPeer(ev2);
  ev2->SetPeer(ev1);

  iworker_->epoll().DelEvent(ev1->fd());
  iworker_->epoll().DelEvent(ev2->fd());

  iworker_->event(ev1->fd()) = ev1;
  iworker_->event(ev2->fd()) = ev2;

  iworker_->epoll().AddEvent(ev1, EPOLLIN);
  iworker_->epoll().AddEvent(ev2, EPOLLIN);
}

ssize_t Handshake::HandleClose() {
  iworker_->AddExceptionEvent(fd_);
  if (req_fd_) close(req_fd_);
  return 0;
}

bool Handshake::HandleData() {
  auto ev1 = this->ToChannel();
  auto ev2 = std::make_shared<Channel>(req_fd_, iworker_);
  ev1->SetPeer(ev2);
  ev2->SetPeer(ev1);

  iworker_->epoll().DelEvent(ev1->fd());

  iworker_->event(ev1->fd()) = ev1;
  iworker_->event(ev2->fd()) = ev2;

  iworker_->epoll().AddEvent(ev1);
  iworker_->epoll().AddEvent(ev2);
  return true;
}

bool Handshake::HandleAuth() {
  char buff[257];  // 1 + 1 + 255
  if (SocketIO::ReadSocket(
          fd_, {reinterpret_cast<uint8_t*>(buff), sizeof(buff)}) < 0)
    return false;

  Auth* auth = reinterpret_cast<Auth*>(buff);
  if (auth->ver != 5) {
    return false;
  }

  for (uint8_t i = 0; i < auth->nmethod; ++i) {
    if (auth->method[i] != 0) continue;

    AuthReply reply = {auth->ver, auth->method[i]};
    if (SocketIO::WriteSocket(
            fd_, {reinterpret_cast<uint8_t*>(&reply), sizeof(reply)}) < 0)
      return false;
    return true;
  }

  return false;
}

bool Handshake::ParseRemoteAddr() {
  char buff[4096];
  if (SocketIO::ReadSocket(
          fd_, {reinterpret_cast<uint8_t*>(buff), sizeof(buff)}) < 0)
    return false;

  Request* req = reinterpret_cast<Request*>(buff);
  if (req->ver != 5) {
    return false;
  }

  if (req->cmd != 1) {  // connect
    return false;
  }

  if (req->address_type == 1) {  // ipv4 addr
    memcpy(reinterpret_cast<void*>(&req_ip_), req->dest_addr, sizeof(req_ip_));
    memcpy(reinterpret_cast<void*>(&req_port_), req->dest_addr + 4,
           sizeof(req_port_));
  } else if (req->address_type == 3) {
    char name_buff[256] = {0};
    memcpy(name_buff, req->dest_addr + 1, req->dest_addr[0]);
    memcpy(&req_port_, req->dest_addr + 1 + req->dest_addr[0],
           sizeof(req_port_));
    auto ret = gethostbyname(name_buff);
    if (!ret) return false;
    for (auto p = ret->h_addr_list; *p; ++p) {
      req_ip_ = *reinterpret_cast<uint32_t*>(*p);
      break;
    }
    fmt::print("target: {}:{}\n",
               inet_ntoa(*reinterpret_cast<in_addr*>(&req_ip_)),
               ntohs(req_port_));
  } else {
    return false;
  }
  return true;
}

bool Handshake::ConnectRemote() {
  sockaddr_in sin;
  sin.sin_family      = AF_INET;
  sin.sin_addr.s_addr = req_ip_;
  sin.sin_port        = req_port_;

  req_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (connect(req_fd_, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) != 0 &&
      errno != EINPROGRESS) {
    close(req_fd_);
    return false;
  }
  return true;
}

bool Handshake::HandleResquest() {
  if (!ParseRemoteAddr()) return false;

  ResquestReplyIpv4 reply{.ver      = 5,
                          .rep      = 0,
                          .rsv      = 0,
                          .atyp     = 1,
                          .bnd_addr = req_ip_,
                          .bnd_port = req_port_};

  if (!ConnectRemote()) {
    reply.rep = 1;
    if (SocketIO::WriteSocket(
            fd_, {reinterpret_cast<uint8_t*>(&reply), sizeof(reply)}) < 0)
      return false;
  }

  remote_conn_             = std::make_shared<RemoteConn>(req_fd_, this);
  iworker_->event(req_fd_) = remote_conn_;
  iworker_->epoll().AddEvent(remote_conn_, EPOLLOUT);

  return true;
}

}  // namespace socks5