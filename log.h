/**
 * @file log.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-21
 */

#pragma once

#include <stdio.h>

#ifdef DEBUG

#define LOG(...) printf(__VA_ARGS__)
#define LOG_ERR(...) fprintf(stderr, __VA_ARGS__)

#else

#define LOG(...)
#define LOG_ERR(...)

#endif
