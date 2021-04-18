/**
 * @file main.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#include "worker.h"

int main(int argc, char** argv) {
  uint16_t port;
  int      c;
  while ((c = getopt(argc, argv, "p:")) != -1) {
    switch (c) {
      case 'p':
        port = static_cast<uint16_t>(std::atoi(optarg));
        break;
      default:
        fmt::print("Invalid opt");
    }
  }
  return std::make_unique<socks5::Worker>(port)->Run();
}