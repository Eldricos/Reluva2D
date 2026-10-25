// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Conservative palette-like resource name/size ranking. A match is not format proof.
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
namespace reluva::stafu {
inline std::string paletteUpper(std::string name){
 for(char& c:name)c=char(std::toupper(static_cast<unsigned char>(c)));
 return name;
}
inline unsigned paletteResourceScore(const std::string& filename,uint64_t size){
 const auto name=paletteUpper(filename);
 unsigned score=0;
 if(name.find("PAL")!=std::string::npos||name.find("CLUT")!=std::string::npos)score+=8;
 if(name.find("COLOR")!=std::string::npos||name.find("COLOUR")!=std::string::npos||
    name.find("VGA")!=std::string::npos)score+=6;
 if(name.find("BRIDGE")!=std::string::npos||name.find("VIEW")!=std::string::npos)score+=3;
 if(size==768||size==1024)score+=7;
 if(size>768&&size<=8192)score+=2;
 return score;
}
}
