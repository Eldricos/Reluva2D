#include "engines/stafu/rm_working_geometry.h"
#include "engines/stafu/rm_section_align.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 using namespace reluva::stafu;
 std::vector<uint8_t> source(640*480*4,0);
 for(size_t y=0;y<480;++y)for(size_t x=0;x<640;++x){
  const size_t p=(y*640+x)*4;
  source[p]=uint8_t((x+y)%256);source[p+3]=255;
 }
 std::vector<uint8_t> standard,reference;
 assert(decodeRmWorkingGeometryRgba(source.data(),source.size(),standard));
 assert(alignRmRgba(source.data(),source.size(),640,480,256,1,reference));
 assert(standard==reference);
 assert(standard[3]==255);
 assert(standard[0]==0); // Top-left of shifted-down left section is black.
 assert(standard[(640+0)*4]==source[384*4]);
 assert(standard[256*4]==source[0]);
 assert(!decodeRmWorkingGeometryRgba(source.data(),source.size()-1,standard));
 return 0;
}
