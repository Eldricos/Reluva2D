// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_database_index.h"
#include <algorithm>
#include <limits>
#include <unordered_map>

namespace reluva::stafu {
namespace {
uint32_t read32(const std::vector<uint8_t>& b, size_t at) {
    return uint32_t(b[at]) | (uint32_t(b[at + 1]) << 8) |
           (uint32_t(b[at + 2]) << 16) | (uint32_t(b[at + 3]) << 24);
}
}
bool ComputerDatabaseIndex::parse(const std::vector<uint8_t>& bytes, std::string& error) {
    // Clear state on failure, including when reusing the same instance.
    primary_.clear();
    secondary_.clear();
    fileSize_ = tableEnd_ = 0;
    error.clear();
    if (bytes.size() < 8) { error = "Database too small for count and offsets"; return false; }
    const uint32_t count = read32(bytes, 0);
    if (!count || uint64_t(count) * 4ull + 4ull > bytes.size()) {
        error = "Primary candidate offset table is empty or truncated"; return false;
    }
    const size_t end = 4 + size_t(count) * 4;
    std::vector<uint32_t> offsets;
    offsets.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t value = read32(bytes, 4 + size_t(i) * 4);
        if (value >= bytes.size()) {
            error = "Primary candidate offset outside database"; return false;
        }
        if (i && value <= offsets.back()) {
            error = "Primary candidate offsets are not strictly increasing"; return false;
        }
        offsets.push_back(value);
    }
    // The observed reference build has nine consecutive references at tableEnd+24.
    // Only recognize this optional pattern when every word exactly matches a
    // nonzero primary offset; do not infer semantics from unrelated bytes.
    std::vector<SecondaryReference> secondary;
    const size_t start = end + 24;
    if (start <= bytes.size() && bytes.size() - start >= 9 * 4) {
        std::unordered_map<uint32_t, uint32_t> lookup;
        for (uint32_t i = 0; i < count; ++i) lookup.emplace(offsets[i], i);
        for (size_t i = 0; i < 9; ++i) {
            const uint32_t target = read32(bytes, start + i * 4);
            const auto it = lookup.find(target);
            if (!target || it == lookup.end()) {
                secondary.clear();
                break;
            }
            secondary.push_back({start + i * 4, target, it->second});
        }
    }
    fileSize_ = bytes.size();
    tableEnd_ = end;
    primary_ = std::move(offsets);
    secondary_ = std::move(secondary);
    return true;
}
bool ComputerDatabaseIndex::offsetAt(size_t index, uint32_t& offset) const {
    if (index >= primary_.size()) return false;
    offset = primary_[index];
    return true;
}
bool ComputerDatabaseIndex::containsRange(size_t offset, size_t length) const {
    return offset <= fileSize_ && length <= fileSize_ - offset;
}
} // namespace reluva::stafu
