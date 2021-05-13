/**
 * @file iworker.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-10
 */
#pragma once

#include <netinet/in.h>

#include <array>
#include <queue>
#include <unordered_map>

#include "epoll.h"
#include "event.h"

namespace socks5 {

enum class Deploy : uint8_t {
  LOCAL  = 1,
  SERVER = 2,
};

enum class Protocol : uint8_t {
  PRIVATE = 1,
  SOCKS5  = 2,
  PASS    = 3,
  SS      = 4,
};

class IWorker {
 public:
  IWorker() : epoll_{events_} {}
  virtual ~IWorker() { events_.clear(); }

  auto& epoll() { return epoll_; }
  auto& events() { return events_; }
  void  AddEvent(const std::shared_ptr<Event>& ev) { events_[ev->fd()] = ev; }
  std::shared_ptr<Event>& event(const int fd) { return events_[fd]; }

  void AddLoopEvent(const int fd) {
    auto ev_it = events_.find(fd);
    if (ev_it == events_.end()) return;
    loop_events_.push(ev_it->second);
  }

  virtual Deploy      deploy() const                  = 0;
  virtual Protocol    protocol() const                = 0;
  virtual bool        encrypt() const                 = 0;
  virtual sockaddr_in static_addr() const             = 0;
  virtual void        AddExceptionEvent(const int fd) = 0;

 public:
  static constexpr uint32_t MAX_EVENTS = 65536;

 protected:
  Epoll<MAX_EVENTS>                               epoll_;
  std::unordered_map<int, std::shared_ptr<Event>> events_;
  std::queue<std::shared_ptr<Event>>              loop_events_;
};

}  // namespace socks5
