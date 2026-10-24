#include "engines/stafu/rm_survey.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 using reluva::stafu::surveyRm;
 assert(surveyRm(nullptr,0).bytes==0);
 const std::vector<uint8_t> bytes={'G','I','F','8','7','a',0,'T','R','P','S',0,'T','S','I','L'};
 const auto r=surveyRm(bytes.data(),bytes.size());
 assert(r.bytes==16&&r.zeroBytes==2);
 assert(r.prefixSize==16&&r.prefix[0]=='G');
 assert(r.gifOffsets.size()==1&&r.gifOffsets[0]==0);
 assert(r.trpsOffsets.size()==1&&r.trpsOffsets[0]==7);
 assert(r.tsilOffsets.size()==1&&r.tsilOffsets[0]==12);
 assert(r.aligned32Count==4);
 const uint8_t tiny[]={0,0,0};
 const auto small=surveyRm(tiny,3);
 assert(small.bytes==3&&small.zeroBytes==3&&small.distinctBytes==1);
 assert(small.aligned32Count==0);
 return 0;
}
