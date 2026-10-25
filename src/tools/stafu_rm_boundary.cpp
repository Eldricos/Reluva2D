// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_boundary_probe.h"
#include "engines/stafu/rm_trailer_index.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
namespace {
constexpr size_t raster=640u*480u;
void report(const char* name,const std::vector<uint8_t>& bytes){
 std::cout<<"RESOURCE "<<name<<" bytes="<<bytes.size()<<" rasterBoundary="<<raster
  <<" trailingBytes="<<(bytes.size()>raster?bytes.size()-raster:0)<<"\n";
 for(int row=474;row<=480;++row){
  const size_t offset=size_t(row)*640;
  const auto s=reluva::stafu::inspectRmBoundaryRow(bytes.data(),bytes.size(),offset,640);
  std::cout<<"ROW "<<row<<" offset="<<offset<<" bytes="<<s.bytes<<" distinct="<<s.distinct
   <<" adjacentEqual="<<s.adjacentEqual<<" highBytes="<<s.highBytes
   <<" first="<<unsigned(s.first)<<" last="<<unsigned(s.last)<<"\n";
 }
 const auto trailer=reluva::stafu::rmByteRuns(bytes.data(),bytes.size(),raster,384);
 std::cout<<"TRAILER_RUNS "<<trailer.size()<<"\n";
 for(const auto& run:trailer)
  std::cout<<" RUN offset="<<run.offset<<" length="<<run.length<<" value="<<unsigned(run.value)<<"\n";
 if(bytes.size()>raster){
  std::cout<<"BOUNDARY lastRaster="<<unsigned(bytes[raster-1])
   <<" firstTrailing="<<unsigned(bytes[raster])
   <<" continuous="<<(bytes[raster-1]==bytes[raster])<<"\n";
  const size_t trailerLen=std::min(size_t(384),bytes.size()-raster);
  for(size_t row=476;row<480;++row){
   std::cout<<"TRAILER_VS_ROW "<<row<<" equalPositions="
    <<reluva::stafu::rmPrefixEqual(bytes.data(),bytes.size(),raster,row*640,trailerLen)
    <<"/"<<trailerLen<<"\n";
  }
 }
 for(const size_t width:{320u,384u,512u,640u,768u,800u,1024u}){
  const auto s=reluva::stafu::inspectRmWidth(bytes.data(),bytes.size(),width);
  std::cout<<"WIDTH "<<width<<" rows="<<s.fullRows<<" remainder="<<s.remainder
   <<" equalBoundaryBytes="<<s.matchingRowBoundaries<<"/"<<s.checkedBoundaries<<"\n";
 }
 std::cout<<"NOTE: BRIDGE.RM and VIEWSCR.RM are treated as distinct images; matching distributions are not expected.\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-boundary <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 for(const char* name:{"BRIDGE.RM","VIEWSCR.RM"}){
  std::vector<uint8_t> bytes;
  if(!resources.read(name,bytes,error,2*1024*1024)){std::cerr<<name<<": "<<error<<"\n";return 1;}
  report(name,bytes);
 }
 return 0;
}
