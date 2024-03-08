/*
 * logging_test.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "base/logging.h"

using namespace ave;

int main(int argc, char const* argv[]) {
  AVE_LOG(LS_INFO) << "log info1";
  AVE_LOG(LS_DEBUG) << "log debug1";
  ave::base::LogMessage::LogToDebug(ave::base::LogSeverity::LS_VERBOSE);
  AVE_LOG(LS_INFO) << "log info2";
  AVE_LOG(LS_INFO) << "log debug2";
  return 0;
}
