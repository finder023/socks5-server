/**
 * @file listener.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#pragma once

#include <signal.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>

#include <queue>

#include "iworker.h"
#include "listener.h"
#include "log.h"

namespace socks5 {

class Worker : public IWorker {
 public:
  Worker(const uint16_t port) : port_{port}, run_{true} { instance_ = this; }
  ~Worker() {}

  bool Init();
  int  Run();

  void AddExceptionEvent(const int fd) override;

  void        WorkerSignal(int sig);
  static void SignalHandler(int sig) { instance_->WorkerSignal(sig); }

 private:
  const uint16_t            port_;
  std::shared_ptr<Listener> listener_;
  bool                      run_;

  static Worker* instance_;
};

}  // namespace socks5
