// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 46: inspect two differing DNAR chunks in translated BRDGRIKR sequences.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace {
uint32_t read32(const uint8_t* p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
void printHex(const std::vector<uint8_t>& data,size_t p,size_t n){
 for(size_t i=0;i<n;++i)std::cout<<(i?" ":"")<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(data[p+i]);
 std::cout<<std::dec<<std::setfill(' ');
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-spr-dnar <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager manager;std::string error;
 if(!manager.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 std::vector<uint8_t> bytes;
 if(!manager.read("BRDGRIKR.SPR",bytes,error)){std::cerr<<error<<"\n";return 1;}
 reluva::stafu::SprSequenceIndex index;
 if(!index.build(bytes,error)||index.records().size()<4){std::cerr<<"Index error: "<<error<<"\n";return 1;}
 const auto& a=index.records()[0];const auto& b=index.records()[3];
 constexpr size_t baseA=16,baseB=39,count=17;
 if(a.events.size()<baseA+count||b.events.size()<baseB+count){std::cerr<<"Missing expected events\n";return 1;}
 size_t dnarPairs=0,differing=0,otherPairs=0;
 for(size_t k=0;k<count;++k){
  const auto& x=a.events[baseA+k];const auto& y=b.events[baseB+k];
  if(x.tag!="DNAR"||y.tag!="DNAR"){++otherPairs;continue;}
  if(x.chunkSize!=16||y.chunkSize!=16||x.payloadSize!=8||y.payloadSize!=8||
     x.chunkOffset>bytes.size()||y.chunkOffset>bytes.size()||
     x.chunkSize>bytes.size()-x.chunkOffset||y.chunkSize>bytes.size()-y.chunkOffset){
   std::cerr<<"Unexpected DNAR bounds/size\n";return 1;
  }
  ++dnarPairs;
  const uint32_t a0=read32(bytes.data()+x.payloadOffset),a1=read32(bytes.data()+x.payloadOffset+4);
  const uint32_t b0=read32(bytes.data()+y.payloadOffset),b1=read32(bytes.data()+y.payloadOffset+4);
  const bool changed=a0!=b0||a1!=b1;differing+=changed;
  std::cout<<"DNAR relativeEvent="<<k<<" events="<<baseA+k<<","<<baseB+k
   <<" chunkOffsets="<<x.chunkOffset<<","<<y.chunkOffset
   <<" wordsA="<<a0<<","<<a1<<" wordsB="<<b0<<","<<b1
   <<" deltas="<<int64_t(b0)-int64_t(a0)<<","<<int64_t(b1)-int64_t(a1)
   <<" changed="<<changed<<"\n";
  std::cout<<"  rawA=";printHex(bytes,x.payloadOffset,8);
  std::cout<<" rawB=";printHex(bytes,y.payloadOffset,8);std::cout<<"\n";
  for(int direction=-1;direction<=1;direction+=2){
   const size_t ix=baseA+k,iy=baseB+k;
   if((direction<0&&(ix==0||iy==0))||(direction>0&&(ix+1>=a.events.size()||iy+1>=b.events.size())))continue;
   const auto& nx=a.events[direction<0?ix-1:ix+1];
   const auto& ny=b.events[direction<0?iy-1:iy+1];
   std::cout<<"  neighbor="<<(direction<0?"before":"after")<<" tags="<<nx.tag<<","<<ny.tag
    <<" sizes="<<nx.chunkSize<<","<<ny.chunkSize;
   reluva::stafu::SprImagePayloadAccessor::View vx,vy;
   if(reluva::stafu::SprImagePayloadAccessor::get(bytes,nx,vx)&&
      reluva::stafu::SprImagePayloadAccessor::get(bytes,ny,vy)&&
      vx.marker==0x30000u&&vy.marker==0x30000u&&vx.size==4&&vy.size==4)
    std::cout<<" values="<<read32(vx.data)<<","<<read32(vy.data)
     <<" shift="<<int64_t(read32(vy.data))-int64_t(read32(vx.data));
   std::cout<<"\n";
  }
 }
 std::cout<<"SUMMARY dnarPairs="<<dnarPairs<<" differing="<<differing<<" otherEventPairs="<<otherPairs<<" expectedTranslation=46048\n";
 std::cout<<"DNAR word interpretations remain unknown; deltas are descriptive, not pointer validation.\n";
 return dnarPairs==2&&differing==2?0:1;
}
