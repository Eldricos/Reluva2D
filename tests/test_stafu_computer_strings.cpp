// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_string_index.h"
#include <cstdio>
#include <string>
#include <vector>
namespace {
int failures=0;
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);++failures;}}while(0)
void set32(std::vector<uint8_t>& b,size_t p,uint32_t v) {
    for(size_t i=0;i<4;++i)b[p+i]=uint8_t(v>>(8*i));
}
}
int main() {
    using I=reluva::stafu::ComputerStringIndex;
    std::vector<uint8_t> b(256,0);
    set32(b,0,1);set32(b,4,0); // base = tableEnd(8)+4 = 12
    set32(b,16,40);set32(b,20,50);set32(b,24,60);
    b[52]='A';b[53]='B';b[54]=0;
    b[62]=0; b[72]=0xff;
    I index;std::string error;
    CHECK(index.parse(b,error));
    CHECK(index.dataBase()==12);
    CHECK(index.entries().size()==1);
    CHECK(index.entries()[0].fields[0].status==I::Status::Text);
    CHECK(index.entries()[0].fields[0].text=="AB");
    CHECK(index.entries()[0].fields[1].status==I::Status::Empty);
    CHECK(index.entries()[0].fields[2].status==I::Status::NonText);
    // Multiline and tabbed DOS text is valid; retain its original bytes.
    b[52]='A';b[53]=0x0d;b[54]='B';b[55]=0x0d;b[56]=0x09;b[57]='C';b[58]=0;
    CHECK(index.parse(b,error));
    CHECK(index.entries()[0].fields[0].status==I::Status::Text);
    CHECK(index.entries()[0].fields[0].text==std::string("A\rB\r\tC"));
    // Preserve CRLF pairs and lone LF exactly as stored.
    b[52]='A';b[53]=0x0d;b[54]=0x0a;b[55]='B';b[56]=0x0a;b[57]='C';b[58]=0;
    CHECK(index.parse(b,error));
    CHECK(index.entries()[0].fields[0].status==I::Status::Text);
    CHECK(index.entries()[0].fields[0].text==std::string("A\\r\\nB\\nC"));
    // Other control bytes and high bytes remain unsupported.
    b[53]=0x0b;
    CHECK(index.parse(b,error));
    CHECK(index.entries()[0].fields[0].status==I::Status::NonText);
    b[53]=0x80;
    CHECK(index.parse(b,error));
    CHECK(index.entries()[0].fields[0].status==I::Status::NonText);
    b[53]=0x0d;
    CHECK(index.parse(b,error));
    CHECK(index.entries()[0].fields[0].status==I::Status::Text);
    set32(b,24,250);
    CHECK(index.parse(b,error));
    CHECK(index.entries()[0].fields[2].status==I::Status::OutOfBounds);
    set32(b,4,250);
    CHECK(!index.parse(b,error));
    CHECK(index.entries().empty());
    CHECK(index.dataBase()==0);
    CHECK(!index.parse({},error));
    return failures?1:0;
}
