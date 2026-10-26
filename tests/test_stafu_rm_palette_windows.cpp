#include "engines/stafu/rm_palette_windows.h"
#include <cassert>
#include <cstdint>
int main(){
 using reluva::stafu::rmPaletteWindows;
 uint8_t vga[384]={},rgb[768]={};
 vga[0]=63;
 rgb[3*128]=255;
 auto w=rmPaletteWindows(vga,384,rgb,768);
 assert(w.size()==129);
 assert(w[0].offset==0&&w[0].exact==127);
 assert(w[128].offset==128&&w[128].exact==128);
 assert(rmPaletteWindows(vga,383,rgb,768).empty());
 assert(rmPaletteWindows(vga,384,rgb,383).empty());
 vga[1]=64;
 assert(rmPaletteWindows(vga,384,rgb,768).empty());
 return 0;
}
