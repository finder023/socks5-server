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

#include "channel.h"

namespace socks5 {

Worker* Worker::instance_;

bool Worker::Init() {
  if (!epoll_.Init()) return false;

  sockaddr_in listen_sin = {0};
  if (!ParseAddress(listen_, &listen_sin)) return false;

  if (deploy() == Deploy::LOCAL) {
    if (!ParseAddress(remote_, &remote_sin_)) return false;
  }

  listener_ = std::make_shared<Listener>(listen_sin, this);
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
    // only channel in loop_events_
    auto ev = std::move(loop_events_.front());
    loop_events_.pop();

    auto status = ev->HandleLoop();
    switch (status) {
      case -1:
        AddExceptionEvent(ev->fd());
        break;
      case 1:
        if (ev->type() == EventType::CHANNEL) {
          auto ch      = std::dynamic_pointer_cast<Channel>(ev);
          auto ch_peer = ch->peer();
          if (!ch_peer) {
            AddExceptionEvent(ev->fd());
            break;
          }
          // unwritable, register epollout for writable
          epoll_.ModEvent(ch, EPOLLIN | EPOLLOUT);
          // epoll do not care epollin
          epoll_.ModEvent(ch_peer, 0);
        }
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

bool Worker::ParseAddress(const std::string& s, sockaddr_in* sin) {
  auto pos = s.find(':');
  if (pos == s.npos) return false;
  std::string ip       = {s.begin(), s.begin() + pos};
  std::string port     = {s.begin() + pos + 1, s.end()};
  sin->sin_addr.s_addr = inet_addr(ip.c_str());
  sin->sin_port        = htons(std::atoi(port.c_str()));
  sin->sin_family      = AF_INET;
  return true;
}

void Worker::WorkerSignal(int sig) {
  LOG("signal [%d][%s] captured\n", sig, strsignal(sig));
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
