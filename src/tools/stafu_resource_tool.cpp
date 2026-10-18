// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only developer verification of the shared STAFU resource manager.
#include "engines/stafu/resource_manager.h"
#include <cstdio>
#include <string>
#include <vector>

int stafuResourceTest(const char* installed, const char* cd, const char* name) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if (!resources.open(installed, cd, error)) {
        std::fprintf(stderr, "resource-test: FAIL opening sources: %s\n", error.c_str());
        return 1;
    }
    reluva::stafu::ResourceLocation location{};
    if (!resources.locate(name, location)) {
        std::fprintf(stderr, "resource-test: FAIL: resource not found: %s\n", name);
        return 1;
    }
    const char* origin = "unknown";
    switch (location.origin) {
    case reluva::stafu::ResourceOrigin::InstalledFile: origin = "installed file"; break;
    case reluva::stafu::ResourceOrigin::MainArchive: origin = "STTNG.ZIP"; break;
    case reluva::stafu::ResourceOrigin::InstallerArchive: origin = "STTNGINS.ZIP"; break;
    }
    std::vector<uint8_t> bytes;
    if (!resources.read(name, bytes, error)) {
        std::fprintf(stderr, "resource-test: FAIL reading %s from %s: %s\n",
                     name, origin, error.c_str());
        return 1;
    }
    std::printf("PASS: %s  source=%s  size=%llu bytes\n", name, origin,
                static_cast<unsigned long long>(bytes.size()));
    return 0;
}
