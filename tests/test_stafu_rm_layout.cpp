#include "engines/stafu/rm_layout_probe.h"
#include <cassert>
#include <cstdint>
int main(){
 using namespace reluva::stafu;
 const uint8_t a[]={1,1,2,2,1,1,2,2,0,0};
 const uint8_t b[]={1,2,2,2,1,1,2,2,0,3};
 const auto rows=scoreRmRows(a,sizeof(a),4,0);
 assert(rows.rows==2&&rows.adjacentCompared==6&&rows.adjacentEqual==2);
 assert(rows.verticalCompared==4&&rows.verticalEqual==4);
 assert(scoreRmRows(nullptr,0,4,0).rows==0);
 assert(scoreRmRows(a,sizeof(a),0,0).rows==0);
 assert(scoreRmRows(a,sizeof(a),4,100).rows==0);
 const auto cmp=compareRmRegions(a,sizeof(a),b,sizeof(b),0,10);
 assert(cmp.compared==10&&cmp.equal==8&&cmp.bothZero==1);
 assert(compareRmRegions(a,sizeof(a),b,sizeof(b),9,5).compared==1);
 assert(compareRmRegions(nullptr,0,b,sizeof(b),0,4).compared==0);
 const auto tail=surveyRmTail(a,sizeof(a),2);
 assert(tail.bytes==2&&tail.zero==2&&tail.distinct==1);
 assert(surveyRmTail(nullptr,0,384).bytes==0);
 return 0;
}
