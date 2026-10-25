// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only standard.pal structure report. No asset bytes are written.
#include "engines/stafu/zip_archive.h"
#include "engines/stafu/standard_pal_probe.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace {
void report(const std::vector<uint8_t>& data,size_t offset){
 const auto s=reluva::stafu::inspectPalTriplets(data.data(),data.size(),offset);
 std::cout<<"TRIPLETS offset="<<offset<<" entries="<<s.entries
  <<" trailingBytes="<<((data.size()-offset)%3)
  <<" distinctColors="<<s.distinctColors
  <<" distinctChannels="<<s.distinctChannelValues
  <<" min="<<s.minChannel<<" max="<<s.maxChannel
  <<" allVga6="<<s.allVga6
  <<" grayscale="<<s.grayscaleEntries
  <<" repeatedAdjacent="<<s.adjacentRepeated
  <<" black="<<s.zeroTriplets<<"\n";
}
}
int main(int argc,char** argv){
 if(argc!=2){std::cerr<<"Usage: reluva-stafu-standard-pal <cd-folder>\n";return 2;}
 reluva::stafu::ZipArchive archive;std::string error;
 if(!archive.open(std::string(argv[1])+"/STTNG.ZIP",error)){std::cerr<<error<<"\n";return 1;}
 std::vector<uint8_t> bytes;
 if(!archive.read("standard.pal",bytes,error,4096)){std::cerr<<error<<"\n";return 1;}
 std::cout<<"RESOURCE standard.pal bytes="<<bytes.size()<<"\n";
 std::cout<<"HEX first "<<std::min<size_t>(48,bytes.size())<<" bytes:";
 for(size_t i=0;i<std::min<size_t>(48,bytes.size());++i)
  std::cout<<" "<<std::hex<<std::setfill('0')<<std::setw(2)<<unsigned(bytes[i]);
 std::cout<<std::dec<<"\n";
 std::cout<<"HEX last "<<std::min<size_t>(24,bytes.size())<<" bytes:";
 for(size_t i=bytes.size()>24?bytes.size()-24:0;i<bytes.size();++i)
  std::cout<<" "<<std::hex<<std::setfill('0')<<std::setw(2)<<unsigned(bytes[i]);
 std::cout<<std::dec<<"\n";
 for(size_t offset=0;offset<3&&offset<bytes.size();++offset)report(bytes,offset);
 std::cout<<"NOTE: Offset variants are alternative interpretations, not proven headers.\n";
 return 0;
}
