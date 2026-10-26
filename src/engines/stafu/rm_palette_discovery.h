// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
namespace reluva::stafu {
inline bool rmPaletteCandidate(std::string_view path){
 std::string s(path);
 std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return char(std::tolower(c));});
 const size_t slash=s.find_last_of("/\\");
 const std::string name=s.substr(slash==std::string::npos?0:slash+1);
 if(name.empty())return false;
 const size_t dot=name.find_last_of('.');
 const std::string ext=dot==std::string::npos?"":name.substr(dot);
 return ext==".pal"||ext==".act"||ext==".clut"||ext==".lut"||
        name.find("palett")!=std::string::npos||
        name.find("colour")!=std::string::npos||
        name.find("color")!=std::string::npos||
        name.find("vga")!=std::string::npos||
        name.find("bridge")!=std::string::npos&&ext==".rm";
}

struct RmPaletteSignature {
 bool rawRgb256=false,vga6_256=false,vga6_128=false,gifGlobal256=false;
};
inline RmPaletteSignature rmPaletteSignature(const uint8_t* data,size_t size){
 RmPaletteSignature s;
 if(!data)return s;
 s.rawRgb256=(size==768);
 if(size==384||size==768){
  bool six=true;
  for(size_t i=0;i<size;++i)if(data[i]>63){six=false;break;}
  s.vga6_128=(size==384&&six);
  s.vga6_256=(size==768&&six);
 }
 if(size>=13&&data[0]=='G'&&data[1]=='I'&&data[2]=='F'&&
    data[3]=='8'&&(data[4]=='7'||data[4]=='9')&&data[5]=='a'&&
    (data[10]&0x80)!=0){
  const size_t entries=size_t(1)<<(size_t(data[10]&7)+1);
  s.gifGlobal256=(entries==256&&size>=13+entries*3);
 }
 return s;
}

}
