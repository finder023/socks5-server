/**
 * @file main.cc
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#include "worker.h"

int main(int argc, char** argv) {
  std::string      listen, deploy_str, protocol_str, encrypt_str, remote_str;
  int              c;
  socks5::Deploy   deploy   = socks5::Deploy::LOCAL;
  socks5::Protocol protocol = socks5::Protocol::SOCKS5;
  bool             encrypt  = false;

  const char* usage =
      "Usage:\n"
      "  -l listen -> ip:port\n"
      "  -r remote -> ip:port\n"
      "  -d deploy -> local/server\n"
      "  -p protocol -> socks5/private/pass\n"
      "  -e encrypt -> true/false\n"
      "  -h help\n";

  while ((c = getopt(argc, argv, "l:r:hd:p:e:")) != -1) {
    switch (c) {
      case 'l':
        listen = optarg;
        break;
      case 'r':
        remote_str = optarg;
        break;
      case 'd':
        deploy_str = optarg;
        if (deploy_str == "local") deploy = socks5::Deploy::LOCAL;
        if (deploy_str == "server") deploy = socks5::Deploy::SERVER;
        break;
      case 'p':
        protocol_str = optarg;
        if (protocol_str == "socks5") protocol = socks5::Protocol::SOCKS5;
        if (protocol_str == "private") protocol = socks5::Protocol::PRIVATE;
        if (protocol_str == "pass") protocol = socks5::Protocol::PASS;
        if (protocol_str == "ss") protocol = socks5::Protocol::SS;
        break;
      case 'e':
        encrypt_str = optarg;
        if (encrypt_str == "true") encrypt = true;
        if (encrypt_str == "false") encrypt = false;
        break;
      case 'h':
        printf("%s", usage);
        break;
      default:
        LOG("Invalid opt\n");
    }
  }
  return std::make_unique<socks5::Worker>(listen, remote_str, deploy, protocol,
                                          encrypt)
      ->Run();
}
