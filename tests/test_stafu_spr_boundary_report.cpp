// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_boundary_report.h"
#include <cassert>
#include <cstdint>
int main() {
    using namespace reluva::stafu;
    const uint8_t a[]={0x0e,0xaa,0xf0};
    const uint8_t b[]={0xff,0xaa,0xf0};
    const SprVerifiedMatch m{true,8,8,16,16,false,false};
    const auto r=describeSprBoundary(a,3,b,3,m);
    assert(r.valid&&r.startA==8&&r.endA==24&&r.leadingA==8);
    assert(r.trailingA==0&&r.reachesEndA&&r.reachesEndB);
    assert(r.hasPriorBits&&!r.priorBitsEqual&&!r.hasNextBits);
    const SprVerifiedMatch mid{true,8,8,8,8,false,false};
    const auto q=describeSprBoundary(a,3,b,3,mid);
    assert(q.valid&&q.hasNextBits&&q.nextBitsEqual&&q.trailingA==8);
    assert(!describeSprBoundary(nullptr,3,b,3,m).valid);
    assert(!describeSprBoundary(a,3,b,3,SprVerifiedMatch{}).valid);
    const SprVerifiedMatch invalid{true,24,8,1,1,false,false};
    assert(!describeSprBoundary(a,3,b,3,invalid).valid);
}
