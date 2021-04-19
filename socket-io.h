/**
 * @file socket-io.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */
#pragma once

#include <errno.h>
#include <fmt/format.h>
#include <stdint.h>
#include <unistd.h>

#include "buffer.h"

namespace socks5 {

class SocketIO {
 public:
  static ssize_t ReadSocket(const int fd, const Buffer buffer) {
    ssize_t n_read = 0;
    while (n_read < buffer.capacity) {
      ssize_t n = read(fd, buffer.memory + n_read, buffer.capacity - n_read);
      if (n < 0) {
        if (errno == EINTR) continue;
        if (errno == EWOULDBLOCK) break;
        fmt::print(stderr, "read socket err. fd = {}, err = {}\n", fd,
                   strerror(errno));
        return -1;
      }
      if (n == 0) {
        fmt::print("read socket closed. fd = {}\n", fd);
        return -1;
      }
      n_read += n;
    }
    return n_read;
  }

  static ssize_t WriteSocket(const int fd, const Buffer buffer) {
    ssize_t n_write = 0;
    while (n_write < buffer.capacity) {
      ssize_t n = write(fd, buffer.memory + n_write, buffer.capacity - n_write);
      if (n < 0) {
        if (errno == EINTR) continue;
        if (errno == EWOULDBLOCK) break;
        fmt::print(stderr, "write socker err. fd = {}, err = {}\n", fd,
                   strerror(errno));
        return -1;
      }
      if (n == 0) {
        fmt::print("write socket closed. fd = {}\n", fd);
        return -1;
      }
      n_write += n;
    }
    return n_write;
  }
};

}  // namespace socks5
