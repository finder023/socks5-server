/**
 * @file socket-io.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */
#pragma once

#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include "buffer.h"
#include "log.h"

namespace socks5 {

class SocketIO {
 public:
  SocketIO(const int fd) : fd_{fd} {}

  ssize_t Read(const Buffer& buffer) {
    ssize_t n_read = 0;
    while (n_read < buffer.capacity) {
      ssize_t n = read(fd_, buffer.memory + n_read, buffer.capacity - n_read);
      if (n < 0) {
        if (errno == EINTR) continue;
        if (errno == EWOULDBLOCK) break;
        LOG(stderr, "read socket err. fd = {}, err = {}\n", fd_,
            strerror(errno));
        return -1;
      }
      if (n == 0) {
        LOG("read socket closed. fd = {}\n", fd_);
        return -1;
      }
      n_read += n;
    }
    return n_read;
  }

  ssize_t Write(const Buffer& buffer) {
    ssize_t n_write = 0;
    while (n_write < buffer.capacity) {
      auto n = write(fd_, buffer.memory + n_write, buffer.capacity - n_write);
      if (n < 0) {
        if (errno == EINTR) continue;
        if (errno == EWOULDBLOCK) break;
        LOG(stderr, "write socker err. fd = {}, err = {}\n", fd_,
            strerror(errno));
        return -1;
      }
      if (n == 0) {
        LOG("write socket closed. fd = {}\n", fd_);
        return -1;
      }
      n_write += n;
    }
    return n_write;
  }

 private:
  const int fd_;
};

}  // namespace socks5
