// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 51: read-only structural survey of two candidate bridge IMG resources.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_img_probe.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace {
void dump(const std::vector<uint8_t>& b,size_t start,size_t limit){
 const size_t end=std::min(b.size(),start+std::min(limit,b.size()-start));
 for(size_t pos=start;pos<end;pos+=16){
  std::cout<<"HEX offset="<<pos<<" ";
  for(size_t i=pos;i<std::min(end,pos+16);++i)
   std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(b[i])<<" ";
  std::cout<<std::dec<<"\n";
 }
}
void inspect(const char* name,const std::vector<uint8_t>& b){
 const auto p=reluva::stafu::probeSprImg(b.data(),b.size());
 std::array<size_t,256> hist{};for(uint8_t v:b)++hist[v];
 size_t distinct=0,zero=hist[0],ff=hist[255];for(size_t v:hist)distinct+=v!=0;
 std::cout<<"RESOURCE name="<<name<<" bytes="<<b.size()<<" distinctByteValues="<<distinct
  <<" zeroBytes="<<zero<<" ffBytes="<<ff<<"\n";
 std::cout<<"HEADER gifSignature="<<p.gifSignature<<" logicalScreen="<<p.logicalScreen
  <<" width="<<p.width<<" height="<<p.height<<" packed="<<unsigned(p.packed)
  <<" background="<<unsigned(p.background)<<" aspect="<<unsigned(p.aspect)
  <<" globalTableBytes="<<p.tableBytes<<" globalTableFits="<<p.globalTableFits
  <<" firstBlockOffset="<<p.firstBlockOffset<<" firstBlock="<<unsigned(p.firstBlock)
  <<" firstBlockValid="<<p.firstBlockValid<<"\n";
 if(!b.empty()){std::cout<<"PREFIX\n";dump(b,0,128);
  if(b.size()>128){std::cout<<"SUFFIX\n";dump(b,b.size()-std::min<size_t>(64,b.size()),64);}
 }
 size_t gifHits=0,imageMarkers=0,extensionMarkers=0,trailerMarkers=0;
 for(size_t i=0;i<b.size();++i){
  imageMarkers+=b[i]==0x2c;extensionMarkers+=b[i]==0x21;trailerMarkers+=b[i]==0x3b;
  if(i<=b.size()-std::min<size_t>(b.size(),6)&&b.size()-i>=6&&
     b[i]=='G'&&b[i+1]=='I'&&b[i+2]=='F'&&b[i+3]=='8'&&
     (b[i+4]=='7'||b[i+4]=='9')&&b[i+5]=='a'){
   ++gifHits;if(gifHits<=16)std::cout<<"GIF_SIGNATURE offset="<<i<<"\n";
  }
 }
 std::cout<<"COUNTS gifSignatures="<<gifHits<<" byte2C="<<imageMarkers
  <<" byte21="<<extensionMarkers<<" byte3B="<<trailerMarkers<<"\n";
 std::cout<<"NOTE Signature and first-block checks do not prove a complete or decodable GIF.\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-bridge-img <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t ok=0,missing=0;
 for(const char* name:{"BRIDGE.IMG","DBRIDGE.IMG"}){
  std::vector<uint8_t> b;
  if(!manager.read(name,b,error)){std::cerr<<"ERROR "<<name<<" "<<error<<"\n";++missing;continue;}
  inspect(name,b);++ok;
 }
 std::cout<<"SUMMARY inspected="<<ok<<" unavailable="<<missing<<"\n";
 return missing?1:0;
}
