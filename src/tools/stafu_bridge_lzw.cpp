// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 53: read-only GIF LZW validation, no image output or renderer changes.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_gif_structure.h"
#include "engines/stafu/spr_gif_lzw.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
namespace {
bool inspect(const char* name,const std::vector<uint8_t>& bytes){
 const auto structure=reluva::stafu::inspectSprGifStructure(bytes.data(),bytes.size());
 if(!structure.valid){std::cout<<"RESOURCE "<<name<<" error="<<structure.error<<"\n";return false;}
 if(structure.images!=1){std::cout<<"RESOURCE "<<name<<" unsupportedImageCount="<<structure.images<<"\n";return false;}
 const auto h=reluva::stafu::probeSprImg(bytes.data(),bytes.size());
 size_t p=h.firstBlockOffset;
 while(p<structure.trailerOffset){
  const uint8_t tag=bytes[p++];
  if(tag==0x21){
   ++p;
   while(p<bytes.size()){const size_t count=bytes[p++];if(!count)break;p+=count;}
   continue;
  }
  if(tag!=0x2c||bytes.size()-p<9)return false;
  const uint16_t w=uint16_t(uint16_t(bytes[p+4])|(uint16_t(bytes[p+5])<<8));
  const uint16_t ht=uint16_t(uint16_t(bytes[p+6])|(uint16_t(bytes[p+7])<<8));
  const uint8_t flags=bytes[p+8];p+=9;
  if(flags&0x80u)p+=size_t(3)*(size_t(1)<<(1+(flags&7u)));
  const unsigned minimum=bytes[p++];
  std::vector<uint8_t> compressed;
  while(p<bytes.size()){
   const size_t count=bytes[p++];if(!count)break;
   compressed.insert(compressed.end(),bytes.begin()+static_cast<std::ptrdiff_t>(p),
                     bytes.begin()+static_cast<std::ptrdiff_t>(p+count));
   p+=count;
  }
  const auto result=reluva::stafu::decodeSprGifLzw(compressed.data(),compressed.size(),minimum,size_t(w)*ht);
  size_t distinct=0;
  bool used[256]{};
  for(uint8_t v:result.indices)used[v]=true;
  for(bool v:used)distinct+=v;
  std::cout<<"RESOURCE "<<name<<" width="<<w<<" height="<<ht<<" interlaced="<<bool(flags&0x40u)
   <<" compressedBytes="<<compressed.size()<<" valid="<<result.valid
   <<" decodedPixels="<<result.indices.size()<<" expectedPixels="<<size_t(w)*ht
   <<" distinctPaletteIndices="<<distinct<<" codes="<<result.codes
   <<" clears="<<result.clears<<" endCode="<<result.sawEnd
   <<" error="<<(result.error.empty()?"none":result.error)<<"\n";
  return result.valid;
 }
 return false;
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-bridge-lzw <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t valid=0,unavailable=0;
 for(const char* name:{"BRIDGE.IMG","DBRIDGE.IMG"}){
  std::vector<uint8_t> bytes;
  if(!manager.read(name,bytes,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++unavailable;continue;}
  valid+=inspect(name,bytes);
 }
 std::cout<<"SUMMARY valid="<<valid<<" unavailable="<<unavailable<<"\n";
 std::cout<<"No RGBA conversion or visual rendering performed.\n";
 return valid==2&&!unavailable?0:1;
}
