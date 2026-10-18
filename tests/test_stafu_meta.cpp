// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/engine.h"
#include <cstdio>
#include <filesystem>
#include <fstream>

namespace reluva { const MetaEngine& stafuMetaEngine(); }

int main() {
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / "reluva_stafu_meta_detection_test";
    fs::remove_all(root);
    fs::create_directories(root);
    const auto& meta = reluva::stafuMetaEngine();
    int failed = 0;
    if (!meta.detect(root).empty()) ++failed;
    for (const char* name : {"STTNG.EXE", "STTNG.OVL", "ASTRO.DB", "COMPUTER.DB"})
        std::ofstream(root / name, std::ios::binary) << "fixture";
    const auto games = meta.detect(root);
    if (games.size() != 1 || games[0].engineId != "stafu" ||
        games[0].gameId != "stafu" || games[0].platform != "dos") ++failed;
    fs::remove(root / "STTNG.OVL");
    if (!meta.detect(root).empty()) ++failed;
    fs::remove_all(root);
    if (failed) std::fprintf(stderr, "STAFU meta detection: %d failures\n", failed);
    return failed ? 1 : 0;
}
