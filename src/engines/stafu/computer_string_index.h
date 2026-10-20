// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace reluva::stafu {
// Observed COMPUTER.DB format: offsets relative to the end of the primary table,
// plus four bytes. This class resolves only the first three references.
class ComputerStringIndex {
public:
    enum class Status { Text, Empty, NonText, Unterminated, OutOfBounds };
    struct Field {
        uint32_t storedOffset=0;
        size_t absoluteOffset=0;
        Status status=Status::OutOfBounds;
        std::string text;
    };
    struct Entry {
        uint32_t primaryOffset=0;
        size_t metadataOffset=0;
        uint16_t headerA=0, headerB=0;
        std::array<Field,3> fields;
    };
    bool parse(const std::vector<uint8_t>& bytes,std::string& error);
    size_t dataBase() const { return base_; }
    const std::vector<Entry>& entries() const { return entries_; }
    static const char* statusName(Status status);
private:
    size_t base_=0;
    std::vector<Entry> entries_;
};
}
