// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace reluva::text {

// Convert UTF-16 little-endian code units to UTF-8. Unpaired surrogates
// become U+FFFD.
std::string utf16leToUtf8(const uint16_t* units, size_t count);

} // namespace reluva::text
