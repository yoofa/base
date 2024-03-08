/*
 * pending_task_flag.cc
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "pending_task_flag.h"
#include <memory>

namespace ave {
namespace base {

// static
std::shared_ptr<PendingTaskFlag> PendingTaskFlag::Create() {
  return std::shared_ptr<PendingTaskFlag>(new PendingTaskFlag(true));
}

void PendingTaskFlag::SetAlive() {
  alive_ = true;
}

void PendingTaskFlag::SetNotAlive() {
  alive_ = false;
}

bool PendingTaskFlag::Alive() const {
  return alive_;
}

}  // namespace base
}  // namespace ave
