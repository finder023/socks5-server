/**
 * @file buffer.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-11
 */
#pragma once

#include <stdint.h>
#include <string.h>

struct Buffer {
  uint8_t* memory;
  uint64_t capacity;
};

template <uint64_t S>
struct Container : public Buffer {
  Container() : Buffer{buff, S}, seek{0}, size{0} {}
  void Shift() {
    memmove(memory, memory + seek, size - seek);
    size = size - seek;
    seek = 0;
  }

  uint64_t seek;
  uint64_t size;
  uint8_t  buff[S];
};
