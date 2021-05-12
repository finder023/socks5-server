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

  auto listen_sin = ParseAddress(listen_);
  if (!listen_sin) return false;

  if (deploy() == Deploy::LOCAL) {
    auto remote_sin = ParseAddress(remote_);
    if (!remote_sin) return false;
    remote_sin_ = *remote_sin;
  }

  listener_ = std::make_shared<Listener>(*listen_sin, this);
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
  signal(SIGPIPE, SignalHandler);

  return true;
}

void Worker::ProcessLoopEvent() {
  while (!loop_events_.empty()) {
    auto ev = std::move(loop_events_.front());
    loop_events_.pop();

    auto status = ev->HandleLoop();
    switch (status) {
      case -1:
        AddExceptionEvent(ev->fd());
        break;
      case 1:
        epoll_.ModEvent(ev, EPOLLIN | EPOLLOUT);
        break;
      default:
        break;
    }
  }
}

int Worker::Run() {
  if (!Init()) return -1;

  LOG("worker init done\n");
  while (run_) {
    epoll_.Wait(-1);

    ProcessLoopEvent();
  }

  return 0;
}

void Worker::AddExceptionEvent(const int fd) {
  epoll_.DelEvent(fd);
  events_.erase(fd);
}

std::optional<sockaddr_in> Worker::ParseAddress(const std::string& s) {
  auto pos = s.find(':');
  if (pos == s.npos) return std::nullopt;
  std::string ip   = {s.begin(), s.begin() + pos};
  std::string port = {s.begin() + pos + 1, s.end()};
  sockaddr_in sin;
  sin.sin_addr.s_addr = inet_addr(ip.c_str());
  sin.sin_port        = htons(std::atoi(port.c_str()));
  sin.sin_family      = AF_INET;
  return sin;
}

void Worker::WorkerSignal(int sig) {
  LOG("signal [{}][{}] captured\n", sig, strsignal(sig));
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
