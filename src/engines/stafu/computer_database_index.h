// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace reluva::stafu {
// Structural observations only: no claim that offsets delimit articles.
class ComputerDatabaseIndex {
public:
    struct SecondaryReference {
        size_t sourceOffset;
        uint32_t targetOffset;
        uint32_t primaryIndex;
    };
    bool parse(const std::vector<uint8_t>& bytes, std::string& error);
    size_t fileSize() const { return fileSize_; }
    size_t tableEnd() const { return tableEnd_; }
    const std::vector<uint32_t>& primaryOffsets() const { return primary_; }
    const std::vector<SecondaryReference>& secondaryReferences() const { return secondary_; }
    bool offsetAt(size_t index, uint32_t& offset) const;
    bool containsRange(size_t offset, size_t length) const;
private:
    size_t fileSize_ = 0;
    size_t tableEnd_ = 0;
    std::vector<uint32_t> primary_;
    std::vector<SecondaryReference> secondary_;
};
} // namespace reluva::stafu
