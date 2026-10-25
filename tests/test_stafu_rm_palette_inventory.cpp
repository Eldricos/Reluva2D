#include "engines/stafu/rm_palette_inventory.h"
#include <cassert>
int main(){
 using namespace reluva::stafu;
 assert(paletteUpper("color.pal")=="COLOR.PAL");
 assert(paletteResourceScore("bridge.pal",768)>paletteResourceScore("bridge.rm",307584));
 assert(paletteResourceScore("vga.dat",1024)>0);
 assert(paletteResourceScore("random.bin",100000)==0);
 assert(paletteResourceScore("data.bin",768)>0);
 return 0;
}
