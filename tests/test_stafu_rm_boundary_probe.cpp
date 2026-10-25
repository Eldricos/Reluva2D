#include "engines/stafu/rm_boundary_probe.h"
#include <cassert>
#include <cstdint>
int main(){
 using namespace reluva::stafu;
 const uint8_t data[]={1,1,2,2,3,3,3,3,4,4};
 const auto r=inspectRmBoundaryRow(data,10,2,4);
 assert(r.offset==2&&r.bytes==4&&r.distinct==2&&r.adjacentEqual==2);
 assert(r.first==2&&r.last==3);
 assert(inspectRmBoundaryRow(data,10,9,4).bytes==1);
 assert(inspectRmBoundaryRow(data,10,10,4).bytes==0);
 assert(rmAdjacentEqual(data,10,0,10)==6);
 const auto w=inspectRmWidth(data,10,4);
 assert(w.width==4&&w.fullRows==2&&w.remainder==2&&w.checkedBoundaries==2);
 assert(w.matchingRowBoundaries==0);
 assert(rmPrefixEqual(data,10,0,2,3)==0);
 assert(rmPrefixEqual(data,10,10,0,3)==0);
 assert(inspectRmWidth(nullptr,10,4).fullRows==0);
 return 0;
}
