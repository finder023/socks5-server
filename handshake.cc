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

namespace socks5 {

int Handshake::Build() {
  if (!HandleAuth()) return -1;
  if (!HandleResquest()) return -1;
  return req_fd_;
}

bool Handshake::HandleAuth() {
  char buff[257];  // 1 + 1 + 255
  auto n = read(fd_, buff, sizeof(buff));
  if (n <= 0) return false;

  Auth* auth = reinterpret_cast<Auth*>(buff);
  if (auth->ver != 5) {
    return false;
  }

  for (uint8_t i = 0; i < auth->nmethod; ++i) {
    if (auth->method[i] != 0) continue;

    AuthReply reply = {auth->ver, auth->method[i]};
    if (write(fd_, reinterpret_cast<const void*>(&reply), sizeof(reply)) !=
        sizeof(reply))
      return false;
    return true;
  }

  return false;
}

bool Handshake::ParseRemoteAddr() {
  char buff[4096];
  auto n = read(fd_, buff, sizeof(buff));
  if (n <= 0) return false;

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

  req_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (connect(req_fd_, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) != 0) {
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
  }
  // reply
  if (write(fd_, &reply, sizeof(reply)) == sizeof(reply)) return true;
  return true;
}

}  // namespace socks5