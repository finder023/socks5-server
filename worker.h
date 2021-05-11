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
  Worker(const std::string& listen, const std::string& remote,
         const Deploy deploy, const Protocol protocol, const bool encrypt)
      : listen_{listen},
        remote_{remote},
        deploy_{deploy},
        protocol_{protocol},
        encrypt_{encrypt},
        run_{true} {
    instance_ = this;
  }
  Worker(const std::string& listen, const Deploy deploy,
         const Protocol protocol, const bool encrypt)
      : Worker{listen, "", deploy, protocol, encrypt} {
    instance_ = this;
  }
  ~Worker() {}

  bool Init();
  int  Run();

  void AddExceptionEvent(const int fd) override;

  void        WorkerSignal(int sig);
  static void SignalHandler(int sig) { instance_->WorkerSignal(sig); }
  Deploy      deploy() const override { return deploy_; }
  Protocol    protocol() const override { return protocol_; }
  bool        encrypt() const override { return encrypt_; }
  sockaddr_in static_addr() const override { return remote_sin_; }

  void ProcessLoopEvent();

 private:
  std::optional<sockaddr_in> ParseAddress(const std::string&);

 private:
  std::shared_ptr<Listener> listener_;
  std::string               listen_;
  std::string               remote_;
  sockaddr_in               remote_sin_;
  Deploy                    deploy_;
  Protocol                  protocol_;
  bool                      encrypt_;
  bool                      run_;
  static Worker*            instance_;
};

}  // namespace socks5
