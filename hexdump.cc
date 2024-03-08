/*
 * hexdump.cc
 * Copyright (C) 2022 youfa.song <vsyfar@gmail.com>
 *
 * Distributed under terms of the GPLv2 license.
 */

#include "hexdump.h"

#include "base/checks.h"
#include "base/logging.h"

namespace ave {
namespace base {

static void appendIndent(std::string& s, int32_t indent) {
  static const char kWhitespace[] =
      "                                        "
      "                                        ";

  AVE_CHECK_LT((size_t)indent, sizeof(kWhitespace));

  s.append(kWhitespace, indent);
}
void hexdump(const void* _data, size_t size, size_t indent) {
  const uint8_t* data = (const uint8_t*)_data;

  size_t offset = 0;
  while (offset < size) {
    std::string line;

    appendIndent(line, indent);

    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%08lx:  ", (unsigned long)offset);

    line.append(tmp);

    for (size_t i = 0; i < 16; ++i) {
      if (i == 8) {
        line.append(" ");
      }
      if (offset + i >= size) {
        line.append("   ");
      } else {
        snprintf(tmp, sizeof(tmp), "%02x ", data[offset + i]);
        line.append(tmp);
      }
    }

    line.append(" ");

    for (size_t i = 0; i < 16; ++i) {
      if (offset + i >= size) {
        break;
      }

      if (isprint(data[offset + i])) {
        line += static_cast<char>(data[offset + i]);
      } else {
        line.append(" ");
      }
    }

    AVE_LOG(LS_INFO) << line.c_str();

    offset += 16;
  }
}
}  // namespace base
} /* namespace ave */
