/*
 * trace_categories.h
 * Copyright (C) 2025 youfa <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#ifndef AVE_BASE_TRACING_TRACE_CATEGORIES_H_
#define AVE_BASE_TRACING_TRACE_CATEGORIES_H_

#ifdef PERFETTO_ENABLE_LEGACY_TRACE_EVENTS
#undef PERFETTO_ENABLE_LEGACY_TRACE_EVENTS
#define PERFETTO_ENABLE_LEGACY_TRACE_EVENTS 1
#endif

#include "third_party/perfetto/include/perfetto/tracing/track_event.h"  // IWYU pragma: export
#include "third_party/perfetto/include/perfetto/tracing/track_event_category_registry.h"
#include "third_party/perfetto/include/perfetto/tracing/track_event_legacy.h"  // IWYU pragma: export

PERFETTO_DEFINE_TEST_CATEGORY_PREFIXES("ave-test");

PERFETTO_DEFINE_CATEGORIES_IN_NAMESPACE(ave, perfetto::Category("ave"));

PERFETTO_USE_CATEGORIES_FROM_NAMESPACE(ave);

#endif  // AVE_BASE_TRACING_TRACE_CATEGORIES_H_
