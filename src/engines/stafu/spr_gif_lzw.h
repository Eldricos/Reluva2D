// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded GIF LZW decoder: LSB-first codes, clear/end codes, 4096-entry dictionary.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace reluva::stafu {
struct SprGifLzwResult {
 bool valid=false, sawEnd=false;
 size_t codes=0, clears=0;
 std::vector<uint8_t> indices;
 std::string error;
};
inline SprGifLzwResult decodeSprGifLzw(const uint8_t* bytes,size_t size,
                                       unsigned minimum,size_t expectedPixels){
 SprGifLzwResult r;
 if((!bytes&&size)||minimum<2||minimum>8||expectedPixels==0||expectedPixels>16*1024*1024){
  r.error="Invalid LZW input or output bound";return r;
 }
 const unsigned clear=1u<<minimum,end=clear+1;
 unsigned next=end+1,width=minimum+1;
 std::array<uint16_t,4096> prefix{};
 std::array<uint8_t,4096> suffix{},stack{};
 size_t bit=0;
 int previous=-1;
 uint8_t previousFirst=0;
 r.indices.reserve(expectedPixels);
 auto fail=[&](const char* why){r.error=why;return r;};
 while(bit<size*8){
  if(size*8-bit<width)return fail("Truncated LZW code");
  unsigned code=0;
  for(unsigned k=0;k<width;++k)code|=unsigned((bytes[(bit+k)/8]>>((bit+k)%8))&1u)<<k;
  bit+=width;++r.codes;
  if(code==clear){next=end+1;width=minimum+1;previous=-1;++r.clears;continue;}
  if(code==end){r.sawEnd=true;break;}
  if(code>=4096||code>next||code==next&&previous<0)return fail("Invalid LZW dictionary reference");
  unsigned walk=code;
  size_t count=0;
  if(code==next){stack[count++]=previousFirst;walk=unsigned(previous);}
  while(walk>=clear){
   if(walk>=next||count>=4095)return fail("Invalid or cyclic LZW prefix chain");
   stack[count++]=suffix[walk];
   walk=prefix[walk];
  }
  if(walk>=clear||count>=4096)return fail("Invalid LZW root");
  const uint8_t first=uint8_t(walk);
  stack[count++]=first;
  if(count>expectedPixels-r.indices.size())return fail("Decoded pixel count exceeds bound");
  while(count)r.indices.push_back(stack[--count]);
  if(previous>=0&&next<4096){
   prefix[next]=uint16_t(previous);suffix[next]=first;++next;
   if(next==(1u<<width)&&width<12)++width;
  }
  previous=int(code);previousFirst=first;
 }
 if(!r.sawEnd)return fail("Missing LZW end code");
 if(r.indices.size()!=expectedPixels)return fail("Decoded pixel count mismatch");
 r.valid=true;return r;
}
}
