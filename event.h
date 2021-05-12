/**
 * @file event.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-11
 */

#pragma once

#include <stdint.h>
#include <unistd.h>

enum class EventType : uint8_t {
  EVENT     = 0,
  HANDSHAKE = 1,
  CONFIRM   = 2,
  CHANNEL   = 3,
};

class Event {
 public:
  Event(const int fd) : fd_{fd} {}
  virtual ~Event() {
    if (fd_) close(fd_);
  }
  const int fd() const { return fd_; }

  virtual const char* name() const { return "event"; }
  virtual EventType   type() const { return EventType::EVENT; }

  virtual ssize_t HandleLoop() { return 0; }

  virtual ssize_t HandleReadable() { return 0; }
  virtual ssize_t HandleWritable() { return 0; }
  virtual ssize_t HandleClose() { return 0; }

 protected:
  int fd_;
};
