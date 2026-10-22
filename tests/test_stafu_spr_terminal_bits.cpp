// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_terminal_bits.h"
#include <cassert>
#include <cstdint>
int main() {
    using reluva::stafu::sprTerminalBitsZero;
    const uint8_t a[]={0xf0,0xa0};
    assert(sprTerminalBitsZero(a,2,0));
    assert(sprTerminalBitsZero(a,2,5));
    assert(!sprTerminalBitsZero(a,2,6));
    assert(!sprTerminalBitsZero(a,2,8));
    assert(!sprTerminalBitsZero(nullptr,2,0));
    assert(!sprTerminalBitsZero(a,0,1));
    const uint8_t b[]={0x01};
    assert(!sprTerminalBitsZero(b,1,1));
}
