// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 54: verify RGBA conversion without writing game assets or images.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_gif_rgba.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-bridge-rgba <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t valid=0,unavailable=0;
 for(const char* name:{"BRIDGE.IMG","DBRIDGE.IMG"}){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  const auto r=reluva::stafu::decodeSprGifRgba(bytes.data(),bytes.size());
  size_t opaque=0,transparent=0;
  for(size_t i=3;i<r.pixels.size();i+=4){opaque+=r.pixels[i]==255;transparent+=r.pixels[i]==0;}
  std::cout<<"RESOURCE "<<name<<" valid="<<r.valid<<" width="<<r.width<<" height="<<r.height
   <<" paletteEntries="<<r.paletteEntries<<" localPalette="<<r.localPalette
   <<" rgbaBytes="<<r.pixels.size()<<" opaquePixels="<<opaque
   <<" transparentPixels="<<transparent<<" error="<<(r.error.empty()?"none":r.error)<<"\n";
  valid+=r.valid;
 }
 std::cout<<"SUMMARY valid="<<valid<<" unavailable="<<unavailable<<"\n";
 std::cout<<"RGBA buffers verified; no renderer integration or image files produced.\n";
 return valid==2&&!unavailable?0:1;
}
