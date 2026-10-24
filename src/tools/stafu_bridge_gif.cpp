// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 52: inspect bridge GIF structure without decoding pixels.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_gif_structure.h"
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-bridge-gif <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t inspected=0,valid=0,unavailable=0;
 for(const char* name:{"BRIDGE.IMG","DBRIDGE.IMG"}){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  ++inspected;
  const auto g=reluva::stafu::inspectSprGifStructure(bytes.data(),bytes.size());
  valid+=g.valid;
  std::cout<<"RESOURCE "<<name<<" bytes="<<bytes.size()<<" valid="<<g.valid
   <<" images="<<g.images<<" extensions="<<g.extensions
   <<" imageSubBlocks="<<g.imageSubBlocks<<" imageDataBytes="<<g.imageDataBytes
   <<" extensionSubBlocks="<<g.extensionSubBlocks<<" extensionDataBytes="<<g.extensionDataBytes
   <<" firstLzwMinimum="<<unsigned(g.firstLzwMinimum)
   <<" trailer="<<g.trailer<<" trailerOffset="<<g.trailerOffset
   <<" trailingBytes="<<g.trailingBytes<<" trailingNonzero="<<g.trailingNonzero
   <<" error="<<(g.error.empty()?"none":g.error)<<"\n";
 }
 std::cout<<"SUMMARY inspected="<<inspected<<" valid="<<valid<<" unavailable="<<unavailable<<"\n";
 std::cout<<"Structural validation does not establish LZW decompression or correct pixels.\n";
 return unavailable||valid!=inspected?1:0;
}
