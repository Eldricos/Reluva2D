// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_trailer_index.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace {
constexpr size_t raster=640u*480u;
void report(const char* name,const std::vector<uint8_t>& bytes){
 std::cout<<"RESOURCE "<<name<<" bytes="<<bytes.size()<<" raster="<<raster
  <<" trailerBytes="<<(bytes.size()>raster?bytes.size()-raster:0)<<"\n";
 const auto runs=reluva::stafu::rmByteRuns(bytes.data(),bytes.size(),raster,384);
 std::cout<<"TRAILER runs="<<runs.size()<<"\n";
 for(const auto& r:runs)
  std::cout<<" RUN offset="<<r.offset<<" length="<<r.length<<" value="<<unsigned(r.value)<<"\n";
 const auto trailer=reluva::stafu::rmIndexRegion(bytes.data(),bytes.size(),raster,384);
 std::cout<<"TRAILER_VALUES distinct="<<trailer.distinct;
 for(size_t i=0;i<256;++i)if(trailer.counts[i])std::cout<<" "<<i<<":"<<trailer.counts[i];
 std::cout<<"\n";
 for(size_t q=0;q<4;++q){
  const auto s=reluva::stafu::rmIndexRegion(bytes.data(),bytes.size(),q*76800,76800);
  std::vector<unsigned> ranked(256);
  for(unsigned i=0;i<256;++i)ranked[i]=i;
  std::sort(ranked.begin(),ranked.end(),[&](unsigned a,unsigned b){
   return s.counts[a]!=s.counts[b]?s.counts[a]>s.counts[b]:a<b;
  });
  std::cout<<"QUARTER "<<q<<" distinct="<<s.distinct<<" top";
  for(size_t i=0;i<8;++i)std::cout<<" "<<ranked[i]<<":"<<s.counts[ranked[i]];
  std::cout<<"\n";
 }
 for(unsigned index:{128u,133u,143u}){
  std::cout<<"INDEX "<<index<<" quarterCounts";
  for(size_t q=0;q<4;++q){
   const auto s=reluva::stafu::rmIndexRegion(bytes.data(),bytes.size(),q*76800,76800);
   std::cout<<" "<<s.counts[index];
  }
  std::cout<<"\n";
 }
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-trailer-index <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 std::vector<uint8_t> bridge,viewscr;
 if(!resources.read("BRIDGE.RM",bridge,error,2*1024*1024)){std::cerr<<error<<"\n";return 1;}
 if(!resources.read("VIEWSCR.RM",viewscr,error,2*1024*1024)){std::cerr<<error<<"\n";return 1;}
 report("BRIDGE.RM",bridge);report("VIEWSCR.RM",viewscr);
 const size_t n=std::min({size_t(384),bridge.size()>raster?bridge.size()-raster:0,
  viewscr.size()>raster?viewscr.size()-raster:0});
 std::cout<<"TRAILER_COMPARE bytes="<<n<<" equalBytes="
  <<reluva::stafu::rmEqualBytes(bridge.data(),bridge.size(),viewscr.data(),viewscr.size(),raster,raster,n)<<"\n";
 std::cout<<"NOTE: Trailer and index distributions are descriptive; they do not establish format semantics.\n";
 return 0;
}
