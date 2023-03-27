/*
 * task.h
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef BASE_TASK_TASK_H
#define BASE_TASK_TASK_H

namespace base {

class Task {
 public:
  virtual ~Task() = default;

  // return true to delete task.
  virtual bool run() = 0;
};

}  // namespace base
#endif /* !BASE_TASK_TASK_H */
