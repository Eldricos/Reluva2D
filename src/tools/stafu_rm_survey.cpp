// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 57: inspect candidate RM scene resources without assuming a format.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_survey.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace {
void printOffsets(const char* label,const std::vector<size_t>& values){
 std::cout<<" "<<label<<"Count="<<values.size()<<" "<<label<<"Offsets=";
 const size_t limit=std::min<size_t>(values.size(),16);
 for(size_t i=0;i<limit;++i){if(i)std::cout<<",";std::cout<<values[i];}
 if(values.size()>limit)std::cout<<",...";
 std::cout<<"\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-survey <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 size_t inspected=0,missing=0;
 for(const char* name:{"BRIDGE.RM","VIEWSCR.RM"}){
  std::vector<uint8_t> bytes;
  if(!resources.read(name,bytes,error)){std::cout<<"RESOURCE "<<name<<" unavailable="<<error<<"\n";++missing;continue;}
  const auto r=reluva::stafu::surveyRm(bytes.data(),bytes.size());
  std::cout<<"RESOURCE "<<name<<" bytes="<<r.bytes<<" zeroBytes="<<r.zeroBytes
   <<" distinctBytes="<<r.distinctBytes<<"\n";
  std::cout<<"PREFIX_HEX";
  for(size_t i=0;i<r.prefixSize;++i)std::cout<<" "<<std::hex<<std::setw(2)
   <<std::setfill('0')<<unsigned(r.prefix[i]);
  std::cout<<std::dec<<"\n";
  std::cout<<"ALIGNED_LE32 firstWords="<<r.aligned32Count<<" inFileRange="
   <<r.aligned32InBounds<<" nondecreasingAdjacent="<<r.aligned32Monotonic
   <<" (uninterpreted candidates)\n";
  printOffsets("gif",r.gifOffsets);
  printOffsets("trps",r.trpsOffsets);
  printOffsets("tsil",r.tsilOffsets);
  ++inspected;
 }
 std::cout<<"SUMMARY inspected="<<inspected<<" unavailable="<<missing<<"\n";
 std::cout<<"No RM fields, image encodings, or scene geometry have been established.\n";
 return inspected==2&&!missing?0:1;
}
