#include "engines/stafu/spr_dnar_packed.h"
#include <cassert>
int main(){
 using reluva::stafu::inspectSprDnarPacked;
 const auto p=inspectSprDnarPacked(20666,10466);
 assert(p.valid&&p.x==206&&p.y==104&&p.halfX==103);
 assert(p.lowX==14&&p.highX==12&&p.lowY==8&&p.highY==6);
 assert(p.lowHalfX==7&&p.highHalfX==6);
 assert(!inspectSprDnarPacked(20667,10466).valid);
 assert(!inspectSprDnarPacked(65,10466).valid);
 assert(!inspectSprDnarPacked(20666,10466,0).valid);
 assert(!inspectSprDnarPacked(20666,10466,17).valid);
 const auto odd=inspectSprDnarPacked(8466,2966,8);
 assert(odd.valid&&odd.x==84&&odd.y==29&&odd.halfX==42);
 return 0;
}
