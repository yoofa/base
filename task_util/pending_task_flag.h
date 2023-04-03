/*
 * pending_task_flag.h
 * Copyright (C) 2023 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef PENDING_TASK_FLAG_H
#define PENDING_TASK_FLAG_H

#include <memory>
namespace base {

class PendingTaskFlag final {
 public:
  static std::shared_ptr<PendingTaskFlag> Create();
  ~PendingTaskFlag() = default;

  void SetAlive();

  void SetNotAlive();

  bool Alive() const;

 protected:
  explicit PendingTaskFlag(bool alive) : alive_(alive) {}

 private:
  bool alive_ = true;
};
}  // namespace base

#endif /* !PENDING_TASK_FLAG_H */
