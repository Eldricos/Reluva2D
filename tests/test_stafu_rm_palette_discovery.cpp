#include "engines/stafu/rm_palette_discovery.h"
#include <cassert>
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
 return 0;
}
