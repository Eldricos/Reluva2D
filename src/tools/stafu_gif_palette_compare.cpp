// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/gif_palette_compare.h"
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-gif-palette-compare <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 const char* names[]={"color.img","BRIDGE.IMG","DBRIDGE.IMG"};
 reluva::stafu::RmCandidatePalette palettes[3];
 for(size_t i=0;i<3;++i){
  std::vector<uint8_t> bytes;
  if(!resources.read(names[i],bytes,error,2*1024*1024)){std::cerr<<names[i]<<": "<<error<<"\n";return 1;}
  palettes[i]=reluva::stafu::extractRmGifPalette(bytes.data(),bytes.size());
  if(!palettes[i].valid){std::cerr<<names[i]<<": "<<palettes[i].error<<"\n";return 1;}
  std::cout<<"PALETTE "<<names[i]<<" entries="<<palettes[i].entries<<"\n";
 }
 for(size_t i=1;i<3;++i){
  const auto d=reluva::stafu::compareGifPalettes(palettes[0],palettes[i]);
  std::cout<<"COMPARE color.img vs "<<names[i]<<" comparedEntries="<<d.comparedEntries
   <<" identicalEntries="<<d.identicalEntries
   <<" differentEntries="<<(d.comparedEntries-d.identicalEntries)
   <<" differentRgbBytes="<<d.differentChannels
   <<" sumAbsoluteRgbDifference="<<d.absoluteChannelDifference
   <<" maximumChannelDifference="<<d.maximumChannelDifference<<"\n";
 }
 std::cout<<"NOTE: Palette similarity does not establish RM palette ownership.\n";
 return 0;
}
