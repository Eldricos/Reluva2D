#include "engines/stafu/rm_palette_scan.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 using namespace reluva::stafu;
 std::vector<uint8_t> data(780,255);
 for(size_t i=0;i<768;++i)data[6+i]=uint8_t(i%64);
 const auto hits=scanVgaPaletteWindows(data.data(),data.size());
 assert(!hits.empty()&&hits.front().offset==6);
 assert(hits.front().distinct==64&&hits.front().maximum==63);
 assert(scanVgaPaletteWindows(nullptr,780).empty());
 assert(scanVgaPaletteWindows(data.data(),767).empty());
 std::vector<uint8_t> noise(768,200);
 assert(scanVgaPaletteWindows(noise.data(),noise.size()).empty());
 assert(equalPaletteRgb(data.data(),data.data(),768));
 assert(!equalPaletteRgb(data.data(),noise.data(),768));
 assert(!equalPaletteRgb(nullptr,data.data(),768));
 return 0;
}
