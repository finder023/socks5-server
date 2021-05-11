/**
 * @file confirm.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-11
 */
#include "confirm.h"

namespace socks5 {

std::shared_ptr<Channel> Confirm::ToChannel() {
  auto channel = std::make_shared<Channel>(fd_, hand_shake_->iworker());
  fd_          = 0;
  return channel;
}

ssize_t Confirm::HandleWritable() {
  uint32_t  val;
  socklen_t len = sizeof(val);
  getsockopt(fd_, SOL_SOCKET, SO_ERROR, &val, &len);
  if (val == 0) {
    hand_shake_->ConfirmRemoteConnection();
    return 0;
  }
  return val;
}

ssize_t Confirm::HandleClose() {
  hand_shake_->iworker()->AddExceptionEvent(fd_);
  return 0;
}

}  // namespace socks5