#include "engines/stafu/gif_palette_compare.h"
#include <cassert>
int main(){
 reluva::stafu::RmCandidatePalette a,b;
 a.valid=b.valid=true;a.entries=b.entries=2;
 a.rgb[0]=10;a.rgb[1]=20;a.rgb[2]=30;
 a.rgb[3]=40;a.rgb[4]=50;a.rgb[5]=60;
 b.rgb[0]=10;b.rgb[1]=20;b.rgb[2]=30;
 b.rgb[3]=44;b.rgb[4]=45;b.rgb[5]=60;
 const auto d=reluva::stafu::compareGifPalettes(a,b);
 assert(d.comparedEntries==2&&d.identicalEntries==1&&d.differentChannels==2);
 assert(d.absoluteChannelDifference==9&&d.maximumChannelDifference==5);
 b.valid=false;
 assert(reluva::stafu::compareGifPalettes(a,b).comparedEntries==0);
 return 0;
}
