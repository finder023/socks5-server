/**
 * @file epoll.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-11
 */
#pragma once

#include <sys/epoll.h>

#include <array>

#include "event.h"
#include "log.h"

template <size_t S>
class Epoll {
 public:
  Epoll(std::array<std::shared_ptr<Event>, S>& events)
      : epoll_fd_{0}, events_{events} {
    bzero(epoll_events_, sizeof(epoll_events_));
  }
  ~Epoll() {}

  bool Init() {
    if ((epoll_fd_ = epoll_create(S)) <= 0) {
      LOG(stderr, "call epoll_create failed. err = {}\n", strerror(errno));
      return false;
    }

    return true;
  }

  bool AddEvent(const std::shared_ptr<Event>& event,
                uint32_t                      flag = EPOLLIN | EPOLLOUT) {
    if (event->fd() >= S || event->fd() <= 0) {
      return false;
    }

    epoll_event ev;
    ev.data.fd = event->fd();
    ev.events  = flag;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event->fd(), &ev) == -1) {
      LOG(stderr, "call epoll_ctl add failed. fd = {}, err = {}\n", event->fd(),
          strerror(errno));
      return false;
    }

    return true;
  }

  bool DelEvent(const int fd) {
    if (fd >= S || fd <= 0) return false;

    epoll_event ev;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev) == -1) {
      LOG(stderr, "call epoll_ctl del failed. fd = {}, err = {}\n", fd,
          strerror(errno));
      return false;
    }

    return true;
  }

  bool ModEvent(const std::shared_ptr<Event>& event, uint32_t flag) {
    if (event->fd() >= S || event->fd() <= 0) {
      return false;
    }

    if (!events_[event->fd()]) {
      return false;
    }

    epoll_event ev;
    ev.data.fd = event->fd();
    ev.events  = flag;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event->fd(), &ev) == -1) {
      LOG(stderr, "call epoll_ctl mod failed. fd = {}, err = {}\n", event->fd(),
          strerror(errno));
      return false;
    }

    return true;
  }

  bool Wait(int timeout) {
    int nfd = epoll_wait(epoll_fd_, epoll_events_, S, timeout);
    if (nfd == -1) {
      LOG(stderr, "call epoll_wait failed. err = {}\n", strerror(errno));
      return false;
    }

    for (int i = 0; i < nfd; ++i) {
      epoll_event* ev    = epoll_events_ + i;
      auto&&       event = events_[ev->data.fd];

      if (event && (ev->events & EPOLLIN)) {
        if (event->HandleReadable() < 0) event->HandleClose();
      }
      if (event && (ev->events & EPOLLOUT)) {
        if (event->HandleWritable() < 0) event->HandleClose();
      }
      if (event && (ev->events & EPOLLHUP || ev->events & EPOLLERR)) {
        event->HandleClose();
      }
    }

    return true;
  }

 private:
  epoll_event                            epoll_events_[S];
  std::array<std::shared_ptr<Event>, S>& events_;
  int                                    epoll_fd_;
};
