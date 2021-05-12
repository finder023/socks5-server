/**
 * @file protocol.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-18
 */

#pragma once

#include <stdint.h>

namespace socks5 {

/*
 +----+----------+----------+
 |VER | NMETHODS | METHODS  |
 +----+----------+----------+
 | 1  |    1     | 1 to 255 |
 +----+----------+----------+
*/
struct Auth {
  uint8_t ver;
  uint8_t nmethod;
  uint8_t method[0];
} __attribute__((packed));

/*
 +----+--------+
 |VER | METHOD |
 +----+--------+
 | 1  |   1    |
 +----+--------+
*/
struct AuthReply {
  uint8_t ver;
  uint8_t method;
} __attribute__((packed));

/*
  +----+-----+-------+------+----------+----------+
  |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
  +----+-----+-------+------+----------+----------+
  | 1  |  1  | X'00' |  1   | Variable |    2     |
  +----+-----+-------+------+----------+----------+
 */

struct Request {
  uint8_t ver;
  uint8_t cmd;
  uint8_t reserved;
  uint8_t address_type;
  uint8_t dest_addr[0];
} __attribute__((packed));

/*
  +----+-----+-------+------+----------+----------+
  |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
  +----+-----+-------+------+----------+----------+
  | 1  |  1  | X'00' |  1   | Variable |    2     |
  +----+-----+-------+------+----------+----------+
*/
struct ResquestReplyIpv4 {
  uint8_t  ver;
  uint8_t  rep;
  uint8_t  rsv;
  uint8_t  atyp;
  uint32_t bnd_addr;
  uint16_t bnd_port;
} __attribute__((packed));

struct ResquestReply {
  uint8_t ver;
  uint8_t rep;
  uint8_t rsv;
  uint8_t atyp;
  uint8_t bnd_addr[0];
} __attribute__((packed));

// private protocol
struct PrivateRequestHeader {
  uint16_t port;
  uint8_t  type;      // 1: ipv4, 2:domain
  uint8_t  addr_len;  // type 1 -> 4. type2 -> variable
  int      fd;
  uint32_t timestamp;
  char     address[0];
} __attribute__((packed));

}  // namespace socks5
