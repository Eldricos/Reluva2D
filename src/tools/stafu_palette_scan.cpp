// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 64: diagnostic only. Read original resources without writing to them.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_candidate_palette.h"
#include "engines/stafu/rm_palette_scan.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
int main(int argc,char** argv){
 if(argc<3){
  std::cerr<<"Usage: reluva-stafu-palette-scan <installed-folder> <cd-folder> [resource-name ...]\n"
           <<"Examples: ... BRIDGE.RM VIEWSCR.RM\n";
  return 2;
 }
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 auto palette=[&](const char* name){
  std::vector<uint8_t> bytes;
  if(!resources.read(name,bytes,error)){std::cerr<<name<<": "<<error<<"\n";return reluva::stafu::RmCandidatePalette{};}
  auto result=reluva::stafu::extractRmGifPalette(bytes.data(),bytes.size());
  std::cout<<"GIF "<<name<<" valid="<<result.valid<<" entries="<<result.entries;
  if(!result.valid)std::cout<<" reason="<<result.error;
  std::cout<<"\n";
  return result;
 };
 const auto bridge=palette("BRIDGE.IMG");
 const auto damaged=palette("DBRIDGE.IMG");
 if(bridge.valid&&damaged.valid){
  const size_t compared=std::min(bridge.entries,damaged.entries)*3;
  size_t differing=0;
  for(size_t i=0;i<compared;++i)if(bridge.rgb[i]!=damaged.rgb[i])++differing;
  std::cout<<"GIF_PALETTE_COMPARE sameEntries="<<(bridge.entries==damaged.entries)
           <<" differingRgbBytes="<<differing<<" comparedRgbBytes="<<compared<<"\n";
 }
 if(argc==3){
  std::cout<<"No resource names supplied for candidate scan. Supply specific filenames from the inventory.\n";
 }
 for(int i=3;i<argc;++i){
  const std::string name=argv[i];
  std::vector<uint8_t> bytes;
  error.clear();
  if(!resources.read(name,bytes,error)){std::cout<<"RESOURCE "<<name<<" ERROR "<<error<<"\n";continue;}
  const auto hits=reluva::stafu::scanVgaPaletteWindows(bytes.data(),bytes.size());
  std::cout<<"RESOURCE "<<name<<" bytes="<<bytes.size()<<" VGA6_CANDIDATES="<<hits.size();
  if(hits.size()==12)std::cout<<" (limit reached)";
  std::cout<<"\n";
  for(const auto& hit:hits)
   std::cout<<"  offset="<<hit.offset<<" distinctChannelValues="<<hit.distinct
            <<" maxChannel="<<unsigned(hit.maximum)<<"\n";
 }
 std::cout<<"NOTE: A VGA6 candidate is a statistical pattern, not an identified game palette.\n";
 return 0;
}
