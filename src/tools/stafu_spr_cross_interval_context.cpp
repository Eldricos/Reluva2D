// SPDX-License-Identifier: GPL-3.0-or-later
// M273: whole-payload occurrences and ordered anchor-chain alternatives.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <iomanip>

namespace {
struct BigCount {
    static constexpr uint64_t base=1000000000ULL;
    std::vector<uint32_t> digits;
    BigCount(uint64_t value=0) {
        while(value){digits.push_back(static_cast<uint32_t>(value%base));value/=base;}
    }
    BigCount& operator+=(const BigCount& other) {
        const size_t count=std::max(digits.size(),other.digits.size());
        digits.resize(count,0);
        uint64_t carry=0;
        for(size_t i=0;i<count;++i){
            uint64_t sum=uint64_t(digits[i])+carry+
                (i<other.digits.size()?other.digits[i]:0);
            digits[i]=static_cast<uint32_t>(sum%base);
            carry=sum/base;
        }
        if(carry)digits.push_back(static_cast<uint32_t>(carry));
        return *this;
    }
    friend BigCount operator*(const BigCount& a,const BigCount& b) {
        BigCount result;
        if(a.digits.empty()||b.digits.empty())return result;
        result.digits.assign(a.digits.size()+b.digits.size(),0);
        for(size_t i=0;i<a.digits.size();++i){
            uint64_t carry=0;
            for(size_t j=0;j<b.digits.size();++j){
                const uint64_t sum=uint64_t(result.digits[i+j])+
                    uint64_t(a.digits[i])*b.digits[j]+carry;
                result.digits[i+j]=static_cast<uint32_t>(sum%base);
                carry=sum/base;
            }
            size_t pos=i+b.digits.size();
            while(carry){
                if(pos==result.digits.size())result.digits.push_back(0);
                const uint64_t sum=uint64_t(result.digits[pos])+carry;
                result.digits[pos]=static_cast<uint32_t>(sum%base);
                carry=sum/base;
                ++pos;
            }
        }
        while(!result.digits.empty()&&result.digits.back()==0)result.digits.pop_back();
        return result;
    }
    bool operator!=(const BigCount& other) const {return digits!=other.digits;}
    std::string str() const {
        if(digits.empty())return "0";
        std::ostringstream out;
        out<<digits.back();
        for(size_t i=digits.size()-1;i>0;--i)
            out<<std::setw(9)<<std::setfill('0')<<digits[i-1];
        return out.str();
    }
};
bool bitAt(const uint8_t* p,size_t bit){return ((p[bit/8]>>(7-(bit%8)))&1u)!=0;}
struct Sample {size_t record,image,offset,size;};
struct Run {size_t start,length;bool equal;};
struct Alignment {int shift;size_t startA,startB,compared,equal;};
bool better(const Alignment& x,const Alignment& y){
    if(!x.compared)return false;
    if(!y.compared)return true;
    const uint64_t lhs=static_cast<uint64_t>(x.equal)*y.compared;
    const uint64_t rhs=static_cast<uint64_t>(y.equal)*x.compared;
    if(lhs!=rhs)return lhs>rhs;
    if(x.compared!=y.compared)return x.compared>y.compared;
    return (x.shift<0?-x.shift:x.shift)<(y.shift<0?-y.shift:y.shift);
}

struct Boundary {
    std::string resource;
    size_t gap,record,imageA,imageB;
    int shift;
    size_t position,runLength;
    bool isStart;
    std::string a,b;
};
bool load(const char* installed,const char* cd,const std::string& name,
          const std::vector<std::vector<size_t>>& gaps){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"SPR_M273_ERROR %s\n",error.c_str());return false;}
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){std::fprintf(stderr,"SPR_M273_ERROR %s\n",error.c_str());return false;}
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){std::fprintf(stderr,"SPR_M273_ERROR %s\n",error.c_str());return false;}
    std::vector<Sample> samples;
    for(size_t ri=0;ri<index.records().size();++ri){
        size_t ordinal=0;
        for(const auto& e:index.records()[ri].events){
            if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
            reluva::stafu::SprImagePayloadAccessor::View v;
            if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
               !(v.marker==0x00030000&&v.size==4))
                samples.push_back({ri,ordinal,v.offset,v.size});
            ++ordinal;
        }
    }
    struct Interval {size_t gap,a0,a1,b0,b1;};
    const std::vector<Interval> intervals=name=="BRDGPICA.SPR"?
        std::vector<Interval>{{3,9908,9920,9967,9989}}:
        std::vector<Interval>{{2,11181,11255,11127,11204},{2,11569,11645,11518,11597}};
    for(size_t ix=0;ix<intervals.size();++ix){
        const auto& v=intervals[ix];const auto& g=gaps[v.gap-1];
        const Sample *sa=nullptr,*sb=nullptr;
        for(const auto& sample:samples){
            if(sample.record==g[0]&&sample.image==g[1])sa=&sample;
            if(sample.record==g[0]&&sample.image==g[2])sb=&sample;
        }
        if(!sa||!sb||v.a0>v.a1||v.b0>v.b1||
           v.a1>sa->size*8||v.b1>sb->size*8){
            std::printf("SPR_M273_ERROR resource=%s interval=%zu invalid-range\n",name.c_str(),ix+1);return false;
        }
        const uint8_t* a=bytes.data()+sa->offset;
        const uint8_t* b=bytes.data()+sb->offset;
        const size_t na=v.a1-v.a0,nb=v.b1-v.b0;
        std::printf("SPR_M273_INTERVAL resource=%s gap=%zu interval=%zu a=%zu bitsA=%zu b=%zu bitsB=%zu deltaBits=%lld\n",
            name.c_str(),v.gap,ix+1,v.a0,na,v.b0,nb,
            static_cast<long long>(nb)-static_cast<long long>(na));
        struct Hit {size_t a,b,len;};
        const std::vector<Hit> hits=name=="BRDGPICA.SPR"?
            std::vector<Hit>{{9908,9977,11}}:
            ix==0?std::vector<Hit>{{11221,11163,13}}:
                  std::vector<Hit>{{11573,11525,13},{11619,11574,13}};
        const auto matches=[&](const uint8_t* source,size_t begin,size_t end,
                               const uint8_t* pattern,size_t offset,size_t length){
            std::vector<size_t> positions;
            if(end<begin||end-begin<length)return positions;
            for(size_t pos=begin;pos<=end-length;++pos){
                bool equal=true;
                for(size_t k=0;k<length;++k)
                    if(bitAt(source,pos+k)!=bitAt(pattern,offset+k)){equal=false;break;}
                if(equal)positions.push_back(pos);
            }
            return positions;
        };
        struct Occurrences {std::vector<size_t> a,b;};
        std::vector<Occurrences> occurrences;
        for(size_t h=0;h<hits.size();++h){
            const auto& hit=hits[h];
            if(hit.a<v.a0||hit.b<v.b0||hit.a+hit.len>v.a1||hit.b+hit.len>v.b1){
                std::printf("SPR_M273_ERROR resource=%s interval=%zu anchor=%zu invalid-range\n",
                    name.c_str(),ix+1,h+1);return false;
            }
            const auto fullA=matches(a,0,sa->size*8,a,hit.a,hit.len);
            const auto fullB=matches(b,0,sb->size*8,a,hit.a,hit.len);
            if(std::find(fullA.begin(),fullA.end(),hit.a)==fullA.end()||
               std::find(fullB.begin(),fullB.end(),hit.b)==fullB.end())return false;
            const size_t surroundA0=v.a0>256?v.a0-256:0;
            const size_t surroundB0=v.b0>256?v.b0-256:0;
            const size_t surroundA1=std::min(sa->size*8,v.a1+256);
            const size_t surroundB1=std::min(sb->size*8,v.b1+256);
            const auto localA=matches(a,surroundA0,surroundA1,a,hit.a,hit.len);
            const auto localB=matches(b,surroundB0,surroundB1,a,hit.a,hit.len);
            const auto countAtShift=[&](long long displacement){
                size_t count=0;
                for(size_t pa:fullA)for(size_t pb:fullB)
                    if(static_cast<long long>(pb)-static_cast<long long>(pa)==displacement)++count;
                return count;
            };
            const long long shift=static_cast<long long>(hit.b)-static_cast<long long>(hit.a);
            std::printf("SPR_M273_ANCHOR resource=%s interval=%zu anchor=%zu a=%zu b=%zu length=%zu displacement=%lld fullOccurrencesA=%zu fullOccurrencesB=%zu surround256A=%zu surround256B=%zu fullPairs=%zu sameDisplacementPairs=%zu\n",
                name.c_str(),ix+1,h+1,hit.a,hit.b,hit.len,shift,
                fullA.size(),fullB.size(),localA.size(),localB.size(),
                fullA.size()*fullB.size(),countAtShift(shift));
            occurrences.push_back({fullA,fullB});
        }
        if(hits.size()==2){
            const auto& x=hits[0];const auto& y=hits[1];
            const size_t expectedA=y.a-x.a,expectedB=y.b-x.b;
            size_t ordered=0,exactSpacing=0,nearSpacing=0;
            size_t sameShift=0;
            for(size_t ax:occurrences[0].a)for(size_t ay:occurrences[1].a){
                if(ay<ax+x.len)continue;
                for(size_t bx:occurrences[0].b)for(size_t by:occurrences[1].b){
                    if(by<bx+x.len)continue;
                    ++ordered;
                    const size_t da=ay-ax,db=by-bx;
                    if(da==expectedA&&db==expectedB)++exactSpacing;
                    if(da+8>=expectedA&&da<=expectedA+8&&
                       db+8>=expectedB&&db<=expectedB+8)++nearSpacing;
                    if(static_cast<long long>(bx)-static_cast<long long>(ax)==
                       static_cast<long long>(by)-static_cast<long long>(ay))++sameShift;
                }
            }
            std::printf("SPR_M273_CHAIN resource=%s interval=%zu anchors=2 expectedSpacingA=%zu expectedSpacingB=%zu orderedPairs=%zu exactSpacingPairs=%zu near8SpacingPairs=%zu constantDisplacementPairs=%zu\n",
                name.c_str(),ix+1,expectedA,expectedB,ordered,exactSpacing,nearSpacing,sameShift);
        }
    }
    return true;
}
}
int stafuSprM273CrossIntervalContext(const char* installed,const char* cd,const char*){
    const std::vector<std::vector<size_t>> picard={
        {5,1,2,3204,4489,3234,4526},{5,1,2,4967,5093,5004,5120},
        {5,1,2,9730,10499,9792,10603},{5,3,4,6342,6591,6464,6713}};
    const std::vector<std::vector<size_t>> data={
        {0,0,1,9973,10390,9966,10356},{0,0,1,10781,11976,10747,11952}};
    if(!load(installed,cd,"BRDGPICA.SPR",picard)||
       !load(installed,cd,"BRDGDATA.SPR",data))return 1;
    std::puts("SPR_M273_SUMMARY intervals=3 scenarios=8 resources=2");
    std::puts("SPR_M273_NOTE Whole-payload and 256-bit-surround occurrence counts; two-anchor ordered chain alternatives. No codec interpretation.");
    return 0;
}
