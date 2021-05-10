/**
 * @file iworker.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-10
 */
#pragma once

#include <netinet/in.h>

#include <array>
#include <unordered_map>

#include "epoll.h"
#include "event.h"

namespace socks5 {

class IWorker {
 public:
  IWorker() : epoll_{events_} {}
  virtual ~IWorker() { events_.clear(); }

  auto& epoll() { return epoll_; }
  auto& events() { return events_; }
  void  AddEvent(const std::shared_ptr<Event>& ev) { events_[ev->fd()] = ev; }
  std::shared_ptr<Event>& event(const int fd) { return events_[fd]; }

  virtual void AddExceptionEvent(const int fd) = 0;

 public:
  static constexpr uint32_t MAX_EVENTS = 65536;

 protected:
  Epoll<MAX_EVENTS> epoll_;
  // std::array<std::shared_ptr<Event>, MAX_EVENTS> events_;
  std::unordered_map<int, std::shared_ptr<Event>> events_;
};

}  // namespace socks5
