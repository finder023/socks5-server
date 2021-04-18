/**
 * @file listener.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#include "worker.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace socks5 {

Worker* Worker::instance_;

bool Worker::Init() {
  if (!epoll_.Init()) return false;

  listener_ = std::make_shared<Listener>(port_, this);
  if (!listener_) return false;
  if (!listener_->StartListener()) return false;

  events_[listener_->fd()] = listener_;
  epoll_.AddEvent(listener_, EPOLLIN);

  // register signal
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  signal(SIGKILL, SignalHandler);
  signal(SIGUSR1, SignalHandler);
  signal(SIGUSR2, SignalHandler);

  return true;
}

int Worker::Run() {
  if (!Init()) return -1;

  fmt::print("worker init done\n");
  while (run_) {
    epoll_.Wait(-1);
  }

  return 0;
}

void Worker::AddExceptionEvent(const int fd) {
  epoll_.DelEvent(fd);
  events_[fd] = nullptr;
}

void Worker::WorkerSignal(int sig) {
  fmt::print("signal [{}] captured\n", sig);
  switch (sig) {
    case SIGKILL:
    case SIGTERM:
    case SIGINT:
      run_ = false;
      break;
    default:
      break;
  }
}

}  // namespace socks5
