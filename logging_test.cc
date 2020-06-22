/*
 * logging_test.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/logging.h"

int main(int /*argc*/, char const* /*argv*/[]) {
#ifndef AVE_ANDROID
  ave::base::LogMessage::LogTimestamps();
  ave::base::LogMessage::LogThreads();
#endif

  AVE_LOG(LS_INFO) << "log info1";
  AVE_LOG(LS_DEBUG) << "log debug1";
  AVE_LOG_IF(LS_INFO, true) << "conditional log";
  AVE_LOG_V(LS_INFO) << "runtime severity log";
  AVE_LOG_F(LS_INFO) << "function log";
  AVE_LOG_IF_F(LS_INFO, true) << "conditional function log";

  AVE_DLOG(LS_INFO) << "debug log";
  AVE_DLOG_IF(LS_INFO, true) << "conditional debug log";
  AVE_DLOG_V(LS_INFO) << "runtime severity debug log";
  AVE_DLOG_F(LS_INFO) << "function debug log";
  AVE_DLOG_IF_F(LS_INFO, true) << "conditional function debug log";

  ave::base::LogMessage::LogToDebug(ave::base::LogSeverity::LS_VERBOSE);
  AVE_LOG(LS_INFO) << "log info2";
  AVE_LOG(LS_DEBUG) << "log debug2";
  return 0;
}
