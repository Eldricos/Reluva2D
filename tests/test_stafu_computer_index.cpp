// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_database_index.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
int failures = 0;
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); ++failures; } } while (0)
void set32(std::vector<uint8_t>& b, size_t at, uint32_t n) {
    for (size_t i = 0; i < 4; ++i) b[at + i] = uint8_t(n >> (8 * i));
}
}
int main() {
    using reluva::stafu::ComputerDatabaseIndex;
    ComputerDatabaseIndex index;
    std::string error;
    std::vector<uint8_t> b(256, 0);
    set32(b, 0, 10);
    for (uint32_t i = 0; i < 10; ++i) set32(b, 4 + i * 4, 0 == i ? 0 : 100 + 10 * i);
    const size_t end = 44;
    for (uint32_t i = 0; i < 9; ++i) set32(b, end + 24 + 4 * i, 110 + 10 * i);
    CHECK(index.parse(b, error));
    CHECK(index.primaryOffsets().size() == 10);
    CHECK(index.tableEnd() == end);
    CHECK(index.secondaryReferences().size() == 9);
    CHECK(index.secondaryReferences()[0].sourceOffset == 68);
    CHECK(index.secondaryReferences()[0].primaryIndex == 1);
    uint32_t offset = 999;
    CHECK(index.offsetAt(9, offset) && offset == 190);
    CHECK(!index.offsetAt(10, offset));
    CHECK(index.containsRange(255, 1));
    CHECK(!index.containsRange(255, 2));
    CHECK(!index.containsRange(257, 0));
    set32(b, end + 24, 77);
    CHECK(index.parse(b, error));
    CHECK(index.secondaryReferences().empty()); // optional pattern
    set32(b, 4 + 4 * 2, 110); // duplicate
    CHECK(!index.parse(b, error));
    CHECK(index.primaryOffsets().empty());
    CHECK(index.secondaryReferences().empty());
    set32(b, 0, 0xffffffffu);
    CHECK(!index.parse(b, error));
    CHECK(!index.parse(std::vector<uint8_t>{}, error));
    set32(b, 0, 10);
    set32(b, 4 + 4 * 2, 256); // out of bounds
    CHECK(!index.parse(b, error));
    return failures ? 1 : 0;
}
