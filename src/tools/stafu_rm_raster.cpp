// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only test of candidate 640x480 raster placements.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_raster_candidate.h"
#include <iostream>
#include <string>
#include <vector>
namespace {
void print(const char* name,const char* layout,const reluva::stafu::RmRasterCandidate& r){
 std::cout<<"CANDIDATE resource="<<name<<" layout="<<layout<<" offset="<<r.offset
  <<" fits="<<r.fits<<" width="<<r.width<<" height="<<r.height<<"\n";
 if(!r.fits)return;
 std::cout<<"  rasterDistinct="<<r.distinct
  <<" horizontalMatches="<<r.horizontalMatches<<"/"<<r.horizontalPairs
  <<" verticalMatches="<<r.verticalMatches<<"/"<<r.verticalPairs<<"\n";
 std::cout<<"  rowBoundaryMatches="<<r.boundaryMatches<<"/"<<r.boundaryPairs
  <<" rowInteriorStartMatches="<<r.interiorMatches<<"/"<<r.interiorPairs<<"\n";
 std::cout<<"  outsideBytes="<<r.outsideBytes<<" outsideZero="<<r.outsideZero
  <<" outsideDistinct="<<r.outsideDistinct<<"\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-raster <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 for(const char* name:{"BRIDGE.RM","VIEWSCR.RM"}){
  std::vector<uint8_t> data;
  if(!resources.read(name,data,error)){std::cerr<<name<<": "<<error<<"\n";return 1;}
  std::cout<<"RESOURCE "<<name<<" bytes="<<data.size()<<"\n";
  print(name,"trailing-384",reluva::stafu::inspectRmRaster(data.data(),data.size(),0));
  print(name,"leading-384",reluva::stafu::inspectRmRaster(data.data(),data.size(),384));
 }
 std::cout<<"NOTE: byte correlations alone cannot establish image geometry, palette, or metadata.\n";
 return 0;
}
