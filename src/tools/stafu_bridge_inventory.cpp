// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only discovery of candidate bridge assets. Names are clues, not proof.
#include "engines/stafu/zip_archive.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace {
std::string lower(std::string s) {
    for(char& c:s)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
bool candidate(const std::string& name) {
    const auto n=lower(name);
    for(const char* token:{"bridge","brdg","enterprise","entd","cockpit","panel","screen","viewscr","command","helm","tactical","picard","riker","data","worf","troi","lcars"})
        if(n.find(token)!=std::string::npos)return true;
    return false;
}
void show(const std::string& origin,const std::string& name,uint64_t size) {
    std::printf("%s\t%s\t%llu\n",origin.c_str(),name.c_str(),
                static_cast<unsigned long long>(size));
}
bool scanArchive(const std::filesystem::path& path,const char* origin,size_t& count) {
    std::error_code ec;
    if(!std::filesystem::is_regular_file(path,ec)) {
        std::fprintf(stderr,"Missing archive: %s\n",path.string().c_str());
        return false;
    }
    reluva::stafu::ZipArchive archive;
    std::string error;
    if(!archive.open(path,error)) {
        std::fprintf(stderr,"Cannot inspect %s: %s\n",path.string().c_str(),error.c_str());
        return false;
    }
    std::printf("# %s: %zu entries\n",origin,archive.size());
    for(const auto& entry:archive.entries()) {
        if(candidate(entry.name)) {
            show(origin,entry.name,entry.uncompressedSize);
            ++count;
        }
    }
    return true;
}
}
int stafuBridgeInventory(const char* installed,const char* cd) {
    const std::filesystem::path installRoot(installed),cdRoot(cd);
    size_t matches=0,loose=0;
    std::puts("# Read-only bridge candidate inventory; name matches do not establish asset purpose.");
    std::puts("origin\tresource\tsize_bytes");
    std::error_code ec;
    if(!std::filesystem::is_directory(installRoot,ec)) {
        std::fprintf(stderr,"Missing installed root: %s\n",installed);
        return 1;
    }
    std::filesystem::directory_iterator it(installRoot,ec),end;
    if(ec)return 1;
    for(;it!=end;it.increment(ec)) {
        if(ec) {
            std::fprintf(stderr,"Directory scan failed: %s\n",ec.message().c_str());
            return 1;
        }
        std::error_code fileError;
        if(!it->is_regular_file(fileError))continue;
        ++loose;
        const auto name=it->path().filename().string();
        if(candidate(name)) {
            const auto size=it->file_size(fileError);
            if(!fileError) {
                show("installed",name,size);
                ++matches;
            }
        }
    }
    if(!scanArchive(cdRoot/"STTNG.ZIP","STTNG.ZIP",matches))return 1;
    // Installer archive is optional, but if present its entries are also candidates.
    if(std::filesystem::is_regular_file(cdRoot/"STTNGINS.ZIP",ec))
        if(!scanArchive(cdRoot/"STTNGINS.ZIP","STTNGINS.ZIP",matches))return 1;
    std::printf("# installed loose files scanned: %zu\n# candidate name matches: %zu\n",loose,matches);
    std::puts("# Next: inspect likely resources using -stafu resource-inspect; do not infer file formats from names.");
    return 0;
}
