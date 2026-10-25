#include "engines/stafu/rm_section_assembly.h"
#include "engines/stafu/rm_section_align.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 using namespace reluva::stafu;
 std::vector<uint8_t> src(4*3*4);
 for(size_t i=0;i<12;++i){src[i*4]=uint8_t(i+1);src[i*4+3]=255;}
 std::vector<uint8_t> sections,baseline;
 assert(assembleRmSectionsRgba(src.data(),src.size(),4,3,2,1,0,sections));
 assert(alignRmRgba(src.data(),src.size(),4,3,2,1,baseline));
 assert(sections==baseline);
 assert(assembleRmSectionsRgba(src.data(),src.size(),4,3,2,0,1,sections));
 assert(sections[0]==3&&sections[4]==4);
 assert(sections[2*4]==0&&sections[2*4+3]==255);
 assert(sections[(4+2)*4]==1);
 assert(!assembleRmSectionsRgba(src.data(),src.size(),4,3,4,0,0,sections));
 assert(!assembleRmSectionsRgba(src.data(),src.size()-1,4,3,2,0,0,sections));
 return 0;
}
