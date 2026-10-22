// SPDX-License-Identifier: GPL-3.0-or-later
// Interactive read-only SPR resource and sequence browser.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_playback_cursor.h"
#include "engines/stafu/spr_image_payload.h"
#include "engines/stafu/spr_bit_survey.h"
#include "engines/stafu/spr_cross_bit_compare.h"
#include "engines/stafu/spr_shift_survey.h"
#include "engines/stafu/spr_shift_verify.h"
#include "engines/stafu/spr_match_boundary.h"
#include "engines/stafu/spr_boundary_report.h"
#include "engines/stafu/spr_bit_reader.h"
#include "engines/stafu/spr_pattern_survey.h"
#include <algorithm>
#include <array>
#include <iomanip>
#include <charconv>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {
bool number(std::string_view s,size_t& n) {
    if(s.empty())return false;
    const auto [end,error]=std::from_chars(s.data(),s.data()+s.size(),n);
    return error==std::errc{}&&end==s.data()+s.size();
}
constexpr const char* resources[]={
    "BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR",
    "BRDGWORF.SPR","brdgdoor.spr","brdgtitl.spr","viewscr.spr"
};
void showEvent(const reluva::stafu::SprSequenceIndex::Event& e,size_t i) {
    std::cout<<"  ["<<i<<"] "<<e.tag<<" offset="<<e.chunkOffset
             <<" size="<<e.chunkSize<<" payload="<<e.payloadSize;
    if(e.hasRawControlValue)std::cout<<" rawEMIT="<<e.rawControlValue;
    if(e.hasImageHeader)
        std::cout<<" header=("<<e.headerA<<","<<e.headerB<<",0x"
                 <<std::hex<<e.marker<<std::dec<<") imageBytes="<<e.imageDataSize
                 <<" imageOffset="<<e.imageDataOffset;
    std::cout<<"\\n";
}
void show(const reluva::stafu::SprSequenceIndex::Record& record) {
    std::cout<<"Record offsets ["<<record.start<<", "<<record.end<<") - "
             <<record.events.size()<<" chunks\\n";
    for(size_t i=0;i<record.events.size();++i)showEvent(record.events[i],i);
}
void inspectPayload(const std::vector<uint8_t>& bytes,
                    const reluva::stafu::SprSequenceIndex::Event& event) {
    reluva::stafu::SprImagePayloadAccessor::View view;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,view)) {
        std::cout<<"No valid image payload for this event.\n";
        return;
    }
    std::array<size_t,256> frequencies{};
    for(size_t i=0;i<view.size;++i)++frequencies[view.data[i]];
    size_t unique=0,zero=frequencies[0],ff=frequencies[255];
    for(size_t count:frequencies)if(count)++unique;
    std::cout<<"Opaque image bytes: offset="<<view.offset<<" length="<<view.size
             <<" uniqueByteValues="<<unique<<" zeros="<<zero<<" ff="<<ff<<"\n";
    const size_t shown=std::min(view.size,size_t(64));
    std::cout<<"First "<<shown<<" bytes (hex):";
    for(size_t i=0;i<shown;++i) {
        if(i%16==0)std::cout<<"\n  ";
        std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
                 <<unsigned(view.data[i])<<" "<<std::dec;
    }
    std::cout<<std::setfill(' ')<<"\n";
    std::cout<<"Byte frequencies and prefixes are observations, not decoded pixels.\n";
}
void inspectBits(const std::vector<uint8_t>& bytes,
                 const reluva::stafu::SprSequenceIndex::Event& event,
                 const char* resource,size_t record) {
    reluva::stafu::SprImagePayloadAccessor::View view;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,view)) {
        std::cout<<"No valid image payload for this event.\\n";return;
    }
    const auto result=reluva::stafu::surveySprBits(view.data,view.size);
    std::cout<<"Bit survey resource="<<resource<<" record="<<record+1
             <<" tag="<<event.tag<<" headerA="<<view.headerA
             <<" headerB="<<view.headerB<<" marker=0x"<<std::hex<<view.marker
             <<std::dec<<" payloadBytes="<<view.size
             <<" examinedBytes="<<result.bytesExamined<<"\\n";
    auto print=[&](const char* name,const reluva::stafu::SprBitSurvey::Orientation& o) {
        std::cout<<"  "<<name<<" ones="<<o.ones<<" transitions="<<o.transitions
                 <<" repeated12="<<o.repeated12<<"/"<<o.windows12
                 <<" aligned8RepeatsByPhase=";
        for(size_t i=0;i<8;++i)std::cout<<(i?",":"")<<o.alignedEightRepeats[i];
        std::cout<<"\\n";
    };
    print("MSB-first",result.msb);
    print("LSB-first",result.lsb);
    std::cout<<"Overlapping windows and repeated tokens are statistical observations, not a decoder.\\n";
}
void inspectCross(const std::vector<uint8_t>& bytes,
                  const reluva::stafu::SprSequenceIndex& index,
                  size_t record,size_t eventNumber,const char* resource) {
    const auto& events=index.records()[record].events;
    reluva::stafu::SprImagePayloadAccessor::View current,previous;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[eventNumber],current)) {
        std::cout<<"Current event has no valid image payload.\\n";return;
    }
    size_t prior=eventNumber;
    while(prior>0) {
        --prior;
        if(reluva::stafu::SprImagePayloadAccessor::get(bytes,events[prior],previous))
            break;
    }
    if(prior==0&&!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[prior],previous)) {
        std::cout<<"No preceding image event in this record.\\n";return;
    }
    if(prior==eventNumber) {
        std::cout<<"No preceding image event in this record.\\n";return;
    }
    const auto r=reluva::stafu::compareSprBits(previous.data,previous.size,current.data,current.size);
    std::cout<<"Cross-bit resource="<<resource<<" record="<<record+1
             <<" previousEvent="<<prior<<" currentEvent="<<eventNumber
             <<" previousTag="<<events[prior].tag<<" currentTag="<<events[eventNumber].tag
             <<" previousBytes="<<previous.size<<" currentBytes="<<current.size
             <<" commonPrefixBytes="<<r.commonPrefix
             <<" examinedBytes="<<r.bytesA<<","<<r.bytesB<<"\\n";
    auto print=[&](const char* order,const std::vector<reluva::stafu::SprCrossBitResult::Trial>& trials) {
        for(const auto& t:trials)
            std::cout<<"  "<<order<<" width="<<t.width<<" phase="<<t.phase
                     <<" matches="<<t.matching<<"/"<<t.tokens
                     <<" shuffledBaseline="<<t.baselineMatching<<"/"<<t.tokens<<"\\n";
    };
    print("MSB",r.msb);print("LSB",r.lsb);
    std::cout<<"Baseline is a deterministic byte shuffle, not statistical significance or proof of token boundaries.\\n";
}
void inspectShift(const std::vector<uint8_t>& bytes,
                  const reluva::stafu::SprSequenceIndex& index,
                  size_t record,size_t eventNumber,const char* resource) {
    const auto& events=index.records()[record].events;
    reluva::stafu::SprImagePayloadAccessor::View current,previous;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[eventNumber],current)) {
        std::cout<<"Current event has no valid image payload.\n";return;
    }
    size_t prior=eventNumber;
    bool found=false;
    while(prior>0) {
        --prior;
        if(reluva::stafu::SprImagePayloadAccessor::get(bytes,events[prior],previous)) {
            found=true;break;
        }
    }
    if(!found) {std::cout<<"No preceding image event in this record.\n";return;}
    const auto r=reluva::stafu::surveySprShifts(previous.data,previous.size,current.data,current.size);
    std::cout<<"Shift survey resource="<<resource<<" record="<<record+1
             <<" previousEvent="<<prior<<" currentEvent="<<eventNumber
             <<" previousTag="<<events[prior].tag<<" currentTag="<<events[eventNumber].tag
             <<" examinedBytes="<<r.bytesA<<","<<r.bytesB<<" maxShiftBytes=256\n";
    auto print=[](const char* label,const reluva::stafu::SprShiftMatch& m,const char* unit) {
        std::cout<<"  "<<label<<" startA="<<m.startA<<" startB="<<m.startB
                 <<" length="<<m.length<<" "<<unit<<"\n";
    };
    print("byte",r.byteMatch,"bytes");
    print("MSB",r.msbMatch,"bits");
    print("LSB",r.lsbMatch,"bits");
    std::cout<<"Longest shifted matches are observations, not proof of encoding or frame reuse.\n";
}
void inspectVerifiedShift(const std::vector<uint8_t>& bytes,
                          const reluva::stafu::SprSequenceIndex& index,
                          size_t record,size_t eventNumber,const char* resource) {
    const auto& events=index.records()[record].events;
    reluva::stafu::SprImagePayloadAccessor::View current,previous;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,events[eventNumber],current)) {
        std::cout<<"Current event has no image payload.\n";return;
    }
    size_t prior=eventNumber;
    bool found=false;
    while(prior>0) {
        --prior;
        if(reluva::stafu::SprImagePayloadAccessor::get(bytes,events[prior],previous)) {
            found=true;break;
        }
    }
    if(!found) {std::cout<<"No preceding image payload.\n";return;}
    const auto survey=reluva::stafu::surveySprShifts(previous.data,previous.size,current.data,current.size);
    std::cout<<"Verified shift resource="<<resource<<" record="<<record+1
             <<" previousEvent="<<prior<<" currentEvent="<<eventNumber<<"\n";
    auto hexExcerpt=[](const char* label,const uint8_t* data,size_t length,size_t start,size_t end) {
        const size_t lo=start>8?start-8:0;
        const size_t hi=std::min(length,end>=length||length-end<8?length:end+8);
        std::cout<<"    "<<label<<" hex ["<<lo<<","<<hi<<"):";
        for(size_t i=lo;i<hi;++i)
            std::cout<<" "<<std::hex<<std::setw(2)<<std::setfill('0')
                     <<unsigned(data[i])<<std::dec;
        std::cout<<std::setfill(' ')<<"\n";
    };
    auto report=[&](const char* label,reluva::stafu::SprShiftMatch match,unsigned order) {
        const auto v=reluva::stafu::verifySprMatch(
            previous.data,previous.size,current.data,current.size,match,order,
            survey.bytesA,survey.bytesB);
        std::cout<<"  "<<label<<" verified="<<(v.valid?"yes":"NO")
                 <<" startA="<<match.startA<<" startB="<<match.startB
                 <<" surveyedLength="<<match.length
                 <<" extendedLength="<<v.extendedLength
                 <<" surveyBoundaryA="<<(v.truncatedA?"yes":"no")
                 <<" surveyBoundaryB="<<(v.truncatedB?"yes":"no")<<"\n";
        if(!v.valid)return;
        const size_t unit=order==0?1:8;
        const size_t startA=match.startA/unit,startB=match.startB/unit;
        const size_t endA=(match.startA+v.extendedLength+unit-1)/unit;
        const size_t endB=(match.startB+v.extendedLength+unit-1)/unit;
        std::cout<<"    displacement="<<(match.startB>=match.startA?"+":"-")
                 <<(match.startB>=match.startA?match.startB-match.startA:match.startA-match.startB)
                 <<(order==0?" bytes":" bits")<<"\n";
        // Show short excerpts around the start and end, without dumping game assets.
        hexExcerpt("A start",previous.data,previous.size,startA,startA);
        hexExcerpt("B start",current.data,current.size,startB,startB);
        hexExcerpt("A end",previous.data,previous.size,endA,endA);
        hexExcerpt("B end",current.data,current.size,endB,endB);
    };
    report("byte",survey.byteMatch,0);
    report("MSB",survey.msbMatch,1);
    report("LSB",survey.lsbMatch,2);
    std::cout<<"Verification checks reported matches against original bytes; it does not decode images.\n";
}
void inspectMocsRecord(const std::vector<uint8_t>& bytes,
                       const reluva::stafu::SprSequenceIndex& index,
                       size_t record,const char* resource) {
    const auto& events=index.records()[record].events;
    reluva::stafu::SprImagePayloadAccessor::View previous,current;
    size_t prior=0;
    bool havePrevious=false;
    size_t pairs=0, longPairs=0, suffixPairs=0;
    std::cout<<"MOCS pair survey resource="<<resource<<" record="<<record+1
             <<" examinedBytesLimit=4096 maxShiftBytes=256\n";
    for(size_t i=0;i<events.size();++i) {
        if(events[i].tag!="MOCS"||
           !reluva::stafu::SprImagePayloadAccessor::get(bytes,events[i],current))
            continue;
        if(havePrevious) {
            const auto result=reluva::stafu::surveySprShifts(
                previous.data,previous.size,current.data,current.size);
            const auto& m=result.msbMatch;
            const auto verified=reluva::stafu::verifySprMatch(
                previous.data,previous.size,current.data,current.size,
                m,1,result.bytesA,result.bytesB);
            const auto displacement=static_cast<long long>(m.startB)-static_cast<long long>(m.startA);
            std::cout<<"  events "<<prior<<" -> "<<i
                     <<" bytes="<<previous.size<<","<<current.size
                     <<" MSB start="<<m.startA<<","<<m.startB
                     <<" shiftBits="<<displacement
                     <<" lengthBits="<<m.length
                     <<" verified="<<(verified.valid?"yes":"NO")
                     <<" byteMatch="<<result.byteMatch.length<<"\n";
            if(verified.valid&&m.length>=64) {
                const auto boundary=reluva::stafu::describeSprBoundary(
                    previous.data,previous.size,current.data,current.size,verified);
                if(boundary.valid) {
                    ++longPairs;
                    if(boundary.reachesEndA||boundary.reachesEndB)++suffixPairs;
                    std::cout<<"    verified boundary A=["<<boundary.startA<<","
                             <<boundary.endA<<") B=["<<boundary.startB<<","
                             <<boundary.endB<<")\n"
                             <<"    unmatched leadingBits="<<boundary.leadingA<<","
                             <<boundary.leadingB<<" trailingBits="<<boundary.trailingA
                             <<","<<boundary.trailingB
                             <<" endsAtPayload="<<(boundary.reachesEndA?"yes":"no")
                             <<","<<(boundary.reachesEndB?"yes":"no")<<"\n";
                    std::cout<<"    priorBits="
                             <<(boundary.hasPriorBits?
                                (boundary.priorBitsEqual?"equal":"different"):"unavailable")
                             <<" nextBits="
                             <<(boundary.hasNextBits?
                                (boundary.nextBitsEqual?"equal":"different"):"unavailable")
                             <<"\n";
                }

                const size_t endA=m.startA+m.length,endB=m.startB+m.length;
                std::cout<<"    A begin "<<reluva::stafu::sprBitWindow(previous.data,previous.size,m.startA,0)
                         <<" end "<<reluva::stafu::sprBitWindow(previous.data,previous.size,endA,0)<<"\n";
                std::cout<<"    B begin "<<reluva::stafu::sprBitWindow(current.data,current.size,m.startB,0)
                         <<" end "<<reluva::stafu::sprBitWindow(current.data,current.size,endB,0)<<"\n";
            }
            ++pairs;
        }
        previous=current;prior=i;havePrevious=true;
    }
    std::cout<<"Compared "<<pairs<<" consecutive MOCS payload pairs within this record.\n";
    std::cout<<"Long verified pairs="<<longPairs<<" with match reaching at least one payload end="<<suffixPairs<<"\n";
    std::cout<<"Match boundaries do not establish insertion/deletion or compression semantics.\n";
}
void inspectBitTokens(const std::vector<uint8_t>& bytes,
                      const reluva::stafu::SprSequenceIndex::Event& event) {
    reluva::stafu::SprImagePayloadAccessor::View view;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,view)) {
        std::cout<<"No valid image payload for this event.\n";return;
    }
    std::string line;
    std::cout<<"Bit offset (0-based, MSB-first, or b to cancel) > "<<std::flush;
    if(!std::getline(std::cin,line)||line=="b")return;
    size_t offset=0;
    if(!number(line,offset)||offset>view.size*8) {
        std::cout<<"Invalid bit offset.\n";return;
    }
    std::cout<<"Token width (1..32 bits) > "<<std::flush;
    if(!std::getline(std::cin,line)||line=="b")return;
    size_t width=0;
    if(!number(line,width)||width<1||width>32) {
        std::cout<<"Invalid token width.\n";return;
    }
    const size_t available=view.size*8-offset;
    const size_t count=std::min(size_t(16),available/width);
    std::cout<<"MSB-first opaque tokens bitOffset="<<offset
             <<" width="<<width<<" count="<<count<<"\n";
    for(size_t i=0;i<count;++i) {
        const size_t pos=offset+i*width;
        uint32_t value=0;
        if(!reluva::stafu::readSprBits(view.data,view.size,pos,
                                      static_cast<unsigned>(width),value))break;
        std::cout<<"  bit="<<pos<<" value="<<value<<" hex=0x"
                 <<std::hex<<value<<std::dec<<" bits=";
        for(size_t j=0;j<width;++j) {
            uint32_t bit=0;
            reluva::stafu::readSprBits(view.data,view.size,pos+j,1,bit);
            std::cout<<bit;
        }
        std::cout<<"\n";
    }
    std::cout<<"Values are unsigned candidates, not decoded symbols or pixels.\n";
}
void inspectPatterns(const std::vector<uint8_t>& bytes,
                     const reluva::stafu::SprSequenceIndex::Event& event) {
    reluva::stafu::SprImagePayloadAccessor::View view;
    if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,event,view)) {
        std::cout<<"No valid image payload for this event.\n";return;
    }
    std::string line;
    std::cout<<"Starting bit offset (MSB-first, b to cancel) > "<<std::flush;
    if(!std::getline(std::cin,line)||line=="b")return;
    size_t start=0;
    if(!number(line,start)||start>view.size*8) {
        std::cout<<"Invalid offset.\n";return;
    }
    const auto repeat=reluva::stafu::longestSprRepeat(view.data,view.size,start);
    std::cout<<"Repeated-period candidate periodBits="<<repeat.period
             <<" consecutiveCopies="<<repeat.repeats
             <<" (search <=64-bit periods, <=512-bit window)\n";
    std::cout<<"width phase tokens distinct adjacentEqual mostFrequent\n";
    for(unsigned width=3;width<=16;++width) {
        for(unsigned phase=0;phase<width;++phase) {
            const auto r=reluva::stafu::surveySprPattern(
                view.data,view.size,start,width,phase);
            if(!r.tokens)continue;
            std::cout<<r.width<<" "<<r.phase<<" "<<r.tokens<<" "
                     <<r.distinct<<" "<<r.adjacentEqual<<" "<<r.mostFrequent<<"\n";
        }
    }
    std::cout<<"Each phase is an exploratory offset relative to the supplied bit position; no token format inferred.\n";
}
void stepRecord(const reluva::stafu::SprSequenceIndex& index,size_t selected,
                const std::vector<uint8_t>& bytes,const char* resource) {
    reluva::stafu::SprPlaybackCursor cursor;
    if(!cursor.bind(index,selected))return;
    std::string input;
    std::cout<<"Stepping record "<<selected+1<<" (raw chunk order, no timing).\\n";
    for(;;) {
        if(const auto* event=cursor.current())showEvent(*event,cursor.position());
        else std::cout<<"[End of record]\\n";
        std::cout<<"n=next, r=restart, l=list all, p=payload, s=bit survey, c=cross compare, h=shift survey, v=verify shift, m=MOCS pairs, t=bit tokens, f=pattern frequencies, b=records, q=quit > "<<std::flush;
        if(!std::getline(std::cin,input)||input=="q"||input=="quit")return;
        if(input=="b"||input=="back")return;
        if(input=="r"||input=="restart"){cursor.restart();continue;}
        if(input=="l"||input=="list"){show(index.records()[selected]);continue;}
        if(input=="p"||input=="payload"){
            if(const auto* event=cursor.current())inspectPayload(bytes,*event);
            else std::cout<<"End of record; no current payload.\n";
            continue;
        }
        if(input=="s"||input=="survey"){
            if(const auto* event=cursor.current())inspectBits(bytes,*event,resource,selected);
            else std::cout<<"End of record; no current payload.\\n";
            continue;
        }
        if(input=="c"||input=="compare"){
            if(cursor.current())inspectCross(bytes,index,selected,cursor.position(),resource);
            else std::cout<<"End of record; no current payload.\\n";
            continue;
        }
        if(input=="h"||input=="shift"){
            if(cursor.current())inspectShift(bytes,index,selected,cursor.position(),resource);
            else std::cout<<"End of record; no current payload.\n";
            continue;
        }
        if(input=="v"||input=="verify"){
            if(cursor.current())inspectVerifiedShift(bytes,index,selected,cursor.position(),resource);
            else std::cout<<"End of record; no current payload.\n";
            continue;
        }
        if(input=="t"||input=="tokens"){
            if(const auto* event=cursor.current())inspectBitTokens(bytes,*event);
            else std::cout<<"End of record; no current payload.\n";
            continue;
        }
        if(input=="f"||input=="patterns"){
            if(const auto* event=cursor.current())inspectPatterns(bytes,*event);
            else std::cout<<"End of record; no current payload.\n";
            continue;
        }
        if(input=="m"||input=="mocs"){
            inspectMocsRecord(bytes,index,selected,resource);
            continue;
        }
        if(input=="n"||input=="next"||input.empty()) {
            if(!cursor.advance())std::cout<<"Already at end. Use r to restart.\\n";
            continue;
        }
        std::cout<<"Unknown command.\\n";
    }
}
}
int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr<<"Usage: reluva-stafu-spr-browser <installed-folder> <cd-folder>\n";
        return 2;
    }
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(argv[1],argv[2],error)) {
        std::cerr<<"SPR browser: "<<error<<"\n";return 1;
    }
    std::cout<<"STAFU SPR sequence browser (read-only). Control semantics unknown.\n";
    for(;;) {
        std::cout<<"\nResources:\n";
        for(size_t i=0;i<sizeof(resources)/sizeof(resources[0]);++i)
            std::cout<<"  "<<i+1<<". "<<resources[i]<<"\n";
        std::cout<<"Select resource number, or q to quit > "<<std::flush;
        std::string input;
        if(!std::getline(std::cin,input)||input=="q"||input=="quit")return 0;
        size_t choice=0;
        if(!number(input,choice)||choice<1||choice>sizeof(resources)/sizeof(resources[0])) {
            std::cout<<"Invalid selection.\n";continue;
        }
        std::vector<uint8_t> bytes;
        if(!manager.read(resources[choice-1],bytes,error)) {
            std::cout<<"Resource unavailable: "<<error<<"\n";continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::cout<<"Invalid SPR resource: "<<error<<"\n";continue;
        }
        for(;;) {
            std::cout<<"\n"<<resources[choice-1]<<": "<<index.records().size()
                     <<" records. Select 1-"<<index.records().size()
                     <<", b=resources, q=quit > "<<std::flush;
            if(!std::getline(std::cin,input)||input=="q"||input=="quit")return 0;
            if(input=="b"||input=="back")break;
            size_t selected=0;
            if(!number(input,selected)||selected<1||selected>index.records().size()) {
                std::cout<<"Invalid record.\n";continue;
            }
            stepRecord(index,selected-1,bytes,resources[choice-1]);
        }
    }
}
