/**
 * @file handshake.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-11
 */

#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "iworker.h"
#include "log.h"
#include "protocol.h"

namespace socks5 {

struct Handshake {
  virtual void                  ConfirmRemoteConnection() = 0;
  virtual IWorker*              iworker()                 = 0;
  virtual PrivateRequestHeader* req_header()              = 0;

  int TcpConnect(const sockaddr_in& sin) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd <= 0) return fd;

    if (connect(fd, reinterpret_cast<const sockaddr*>(&sin), sizeof(sin)) !=
            0 &&
        errno != EINPROGRESS) {
      close(fd);
      return -1;
    }
    return fd;
  }

  uint32_t QueryDNS(const char* domain, const uint32_t len) {
    char     name_buff[256] = {0};
    uint32_t result;

    memcpy(name_buff, domain, len);
    auto ret = gethostbyname(name_buff);
    if (!ret) return -1;
    for (auto p = ret->h_addr_list; *p; ++p) {
      result = *reinterpret_cast<uint32_t*>(*p);
      break;
    }
    return result;
  }
};

}  // namespace socks5