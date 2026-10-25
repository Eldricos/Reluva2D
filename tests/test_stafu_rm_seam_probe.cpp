#include "engines/stafu/rm_seam_probe.h"
#include <cassert>
#include <cstdint>
int main(){
 using namespace reluva::stafu;
 const uint8_t a[]={1,2,3,4, 5,6,3,8, 9,10,7,12};
 const auto e=rmEdgeEvidence(a,12,4,3,2);
 assert(e.pairs==1&&e.sameRow==0&&e.priorRow==0&&e.nextRow==0);
 const auto s=rmSeamEvidence(a,12,4,3,2);
 assert(s.rows==3&&s.rawEqual==0&&s.alignedPreviousRowEqual==0);
 assert(rmEdgeEvidence(nullptr,12,4,3,2).pairs==0);
 assert(rmSeamEvidence(a,7,4,3,2).rows==0);
 return 0;
}
