/**
 * @file confirm.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-05-11
 */
#include "confirm.h"

#include "encryptor.h"

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
  if (val != 0) return -1;

  IWorker* iworker = hand_shake_->iworker();
  if (iworker->deploy() == Deploy::LOCAL &&
      (iworker->protocol() == Protocol::SOCKS5 ||
       iworker->protocol() == Protocol::SS)) {
    auto     req_header = hand_shake_->req_header();
    uint64_t len        = sizeof(PrivateRequestHeader) + req_header->addr_len;
    uint8_t  req_buff[len];
    memcpy(req_buff, req_header, len);
    Buffer tmp{req_buff, len};

    if (hand_shake_->iworker()->encrypt()) {
      Encryptor{}.NaiveEncrypt(tmp);
    }

    if (SocketIO(fd_).Write(tmp) < 0) return -1;
    LOG("write %d target: %s\n", fd_, (char*)req_header->address);
  }

  hand_shake_->ConfirmRemoteConnection();
  return 0;
}

ssize_t Confirm::HandleClose() {
  hand_shake_->iworker()->AddExceptionEvent(fd_);
  return 0;
}

}  // namespace socks5