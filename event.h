/**
 * @file event.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-11
 */

#pragma once

#include <unistd.h>

class Event {
 public:
  Event(const int fd) : fd_{fd} {}
  virtual ~Event() {
    if (fd_) close(fd_);
  }
  const int fd() const { return fd_; }
  void      ClearFd() { fd_ = 0; }

  virtual ssize_t HandleReadable() { return 0; }
  virtual ssize_t HandleWritable() { return 0; }
  virtual ssize_t HandleClose() { return 0; }

 protected:
  int fd_;
};
