// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only real-resource integration for the provisional COMPUTER.DB index.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <cstdio>
#include <string>
#include <vector>

int stafuComputerIndex(const char* installed, const char* cd) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if (!resources.open(installed, cd, error)) {
        std::fprintf(stderr, "computer-index: resource manager: %s\n", error.c_str());
        return 1;
    }
    std::vector<uint8_t> bytes;
    if (!resources.read("computer.db", bytes, error)) {
        std::fprintf(stderr, "computer-index: reading computer.db: %s\n", error.c_str());
        return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if (!index.parse(bytes, error)) {
        std::fprintf(stderr, "computer-index: parse failed: %s\n", error.c_str());
        return 1;
    }
    std::printf("Resource: computer.db\nBytes: %zu\n", bytes.size());
    std::printf("Primary candidate entries: %zu\n", index.primaryOffsets().size());
    std::printf("Primary candidate table ends at: 0x%08zx\n", index.tableEnd());
    std::printf("Secondary pattern matches: %zu of 9\n", index.secondaryReferences().size());
    for (const auto& ref : index.secondaryReferences())
        std::printf("  file 0x%08zx -> primary[%u] = 0x%08x\n",
                    ref.sourceOffset, ref.primaryIndex, ref.targetOffset);
    std::puts("PASS: candidate index structure parsed; record semantics remain unverified.");
    return 0;
}
