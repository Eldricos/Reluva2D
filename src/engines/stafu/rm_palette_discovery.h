// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cctype>
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
}
