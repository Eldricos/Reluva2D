// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Visually validated working RM geometry, not a claim about the original format.
#include "engines/stafu/rm_section_assembly.h"
namespace reluva::stafu {
inline bool decodeRmWorkingGeometryRgba(const uint8_t* source,size_t length,
                                        std::vector<uint8_t>& out){
 return assembleRmSectionsRgba(source,length,640,480,384,1,0,out);
}
}
