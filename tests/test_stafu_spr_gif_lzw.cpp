#include "engines/stafu/spr_gif_lzw.h"
#include <cassert>
#include <cstdint>
int main(){
 using reluva::stafu::decodeSprGifLzw;
 const uint8_t encoded[]={0x44,0x01}; // clear(4), literal(0), end(5), 3-bit LSB codes
 const auto ok=decodeSprGifLzw(encoded,2,2,1);
 assert(ok.valid&&ok.sawEnd&&ok.clears==1&&ok.indices.size()==1&&ok.indices[0]==0);
 assert(!decodeSprGifLzw(encoded,2,2,2).valid);
 assert(!decodeSprGifLzw(encoded,1,2,1).valid);
 assert(!decodeSprGifLzw(encoded,2,1,1).valid);
 assert(!decodeSprGifLzw(nullptr,2,2,1).valid);
 assert(!decodeSprGifLzw(encoded,2,2,0).valid);
 const uint8_t invalid[]={0x3c,0x01}; // clear(4), invalid code(7), end(4-bit stream)
 assert(!decodeSprGifLzw(invalid,2,2,1).valid);
 return 0;
}
