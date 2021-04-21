/**
 * @file log.h
 * @author yaolongliu (liuyaolong023@163.com)
 * @brief
 * @date 2021-04-21
 */

#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>

#ifdef DEBUG

#define LOG(...) fmt::print(__VA_ARGS__)

#else

#define LOG(...)

#endif
