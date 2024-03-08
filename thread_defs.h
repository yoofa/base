/*
 * thread_defs.h
 * Copyright (C) 2023 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef THREAD_DEFS_H
#define THREAD_DEFS_H

#if defined(__cplusplus)
extern "C" {
#endif

enum {
  AVE_PRIORITY_LOWEST = 19,

  /* use for background tasks */
  AVE_PRIORITY_BACKGROUND = 10,

  /* most threads run at normal priority */
  AVE_PRIORITY_NORMAL = 0,

  /* all normal video threads */
  AVE_PRIORITY_VIDEO = -10,

  /* all normal audio threads */
  AVE_PRIORITY_AUDIO = -16,

  /* should never be used in practice. regular process might not
   * be allowed to use this level */
  AVE_PRIORITY_HIGHEST = -20,

  AVE_PRIORITY_DEFAULT = AVE_PRIORITY_NORMAL,
};

#if defined(__cplusplus)
}
#endif

#endif /* !THREAD_DEFS_H */
