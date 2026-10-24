#pragma once
#include <cstdint>
namespace reluva::stafu {
struct SprDnarPacked { bool valid=false; uint32_t x=0,y=0,halfX=0,lowX=0,lowY=0,lowHalfX=0,highX=0,highY=0,highHalfX=0; };
inline SprDnarPacked inspectSprDnarPacked(uint32_t a,uint32_t b,unsigned bits=4){
 SprDnarPacked r;
 if(a<66||b<66||(a-66)%100||(b-66)%100||bits==0||bits>16)return r;
 r.valid=true;r.x=(a-66)/100;r.y=(b-66)/100;r.halfX=r.x/2;
 const uint32_t mask=(uint32_t(1)<<bits)-1;
 r.lowX=r.x&mask;r.lowY=r.y&mask;r.lowHalfX=r.halfX&mask;
 r.highX=r.x>>bits;r.highY=r.y>>bits;r.highHalfX=r.halfX>>bits;
 return r;
}
}
