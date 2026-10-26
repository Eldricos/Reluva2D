#include "engines/stafu/rm_palette_discovery.h"
#include <cassert>
#include <cstdint>
int main(){
 using reluva::stafu::rmPaletteCandidate;
 assert(rmPaletteCandidate("standard.pal"));
 assert(rmPaletteCandidate("SUBDIR\\BRIDGE.PAL"));
 assert(rmPaletteCandidate("palettes/room.CLUT"));
 assert(rmPaletteCandidate("COLOR.IMG"));
 assert(rmPaletteCandidate("BRIDGE.RM"));
 assert(!rmPaletteCandidate("VIEWSCR.RM"));
 assert(!rmPaletteCandidate("bridge.img"));
 assert(!rmPaletteCandidate("computer.db"));
 assert(!rmPaletteCandidate(""));
 uint8_t raw[768]={};
 auto sig=reluva::stafu::rmPaletteSignature(raw,sizeof(raw));
 assert(sig.rawRgb256&&sig.vga6_256&&!sig.gifGlobal256);
 raw[4]=64;
 sig=reluva::stafu::rmPaletteSignature(raw,sizeof(raw));
 assert(sig.rawRgb256&&!sig.vga6_256);
 uint8_t gif[781]={};
 gif[0]='G';gif[1]='I';gif[2]='F';gif[3]='8';gif[4]='9';gif[5]='a';
 gif[10]=0x87;
 assert(reluva::stafu::rmPaletteSignature(gif,sizeof(gif)).gifGlobal256);
 assert(!reluva::stafu::rmPaletteSignature(gif,780).gifGlobal256);
 assert(!reluva::stafu::rmPaletteSignature(nullptr,0).rawRgb256);
 return 0;
}
