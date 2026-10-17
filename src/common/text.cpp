// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "common/text.h"

namespace reluva::text {

static void appendUtf8(std::string& out, uint32_t cp) {
    if (cp < 0x80) {
        out += char(cp);
    } else if (cp < 0x800) {
        out += char(0xc0 | (cp >> 6));
        out += char(0x80 | (cp & 0x3f));
    } else if (cp < 0x10000) {
        out += char(0xe0 | (cp >> 12));
        out += char(0x80 | ((cp >> 6) & 0x3f));
        out += char(0x80 | (cp & 0x3f));
    } else {
        out += char(0xf0 | (cp >> 18));
        out += char(0x80 | ((cp >> 12) & 0x3f));
        out += char(0x80 | ((cp >> 6) & 0x3f));
        out += char(0x80 | (cp & 0x3f));
    }
}

std::string utf16leToUtf8(const uint16_t* units, size_t count) {
    std::string out;
    out.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        uint32_t u = units[i];
        if (u >= 0xd800 && u < 0xdc00 && i + 1 < count && units[i + 1] >= 0xdc00 && units[i + 1] < 0xe000) {
            appendUtf8(out, 0x10000 + ((u - 0xd800) << 10) + (units[i + 1] - 0xdc00));
            ++i;
        } else if (u >= 0xd800 && u < 0xe000) {
            appendUtf8(out, 0xfffd);
        } else {
            appendUtf8(out, u);
        }
    }
    return out;
}

} // namespace reluva::text
