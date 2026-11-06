// SPDX-License-Identifier: GPL-3.0-or-later
// M271: anchor compatibility under alternative insertion/deletion costs.
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
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"SPR_M271_ERROR %s\n",error.c_str());return false;}
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){std::fprintf(stderr,"SPR_M271_ERROR %s\n",error.c_str());return false;}
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){std::fprintf(stderr,"SPR_M271_ERROR %s\n",error.c_str());return false;}
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
            std::printf("SPR_M271_ERROR resource=%s interval=%zu invalid-range\n",name.c_str(),ix+1);return false;
        }
        const uint8_t* a=bytes.data()+sa->offset;
        const uint8_t* b=bytes.data()+sb->offset;
        const size_t na=v.a1-v.a0,nb=v.b1-v.b0;
        std::printf("SPR_M271_INTERVAL resource=%s gap=%zu interval=%zu a=%zu bitsA=%zu b=%zu bitsB=%zu deltaBits=%lld\n",
            name.c_str(),v.gap,ix+1,v.a0,na,v.b0,nb,
            static_cast<long long>(nb)-static_cast<long long>(na));
        struct Hit {size_t a,b,len;};
        const std::vector<Hit> hits=name=="BRDGPICA.SPR"?
            std::vector<Hit>{{9908,9977,11}}:
            ix==0?std::vector<Hit>{{11221,11163,13}}:
                  std::vector<Hit>{{11573,11525,13},{11619,11574,13}};
        const size_t scenarios=size_t(1)<<hits.size();
        struct Score {int cost=0;BigCount paths=BigCount(1);bool valid=true;};
        const auto scoreSegment=[&](size_t a0,size_t a1,size_t b0,size_t b1,int substitution,int gapCost)->Score {
            const size_t m=a1-a0,n=b1-b0,w=n+1;
            const auto at=[w](size_t i,size_t j){return i*w+j;};
            std::vector<int> dp((m+1)*w,0);
            std::vector<BigCount> count((m+1)*w);
            count[0]=BigCount(1);
            for(size_t i=0;i<=m;++i)for(size_t j=0;j<=n;++j){
                if(i==0&&j==0)continue;
                int best=static_cast<int>(m+n)*4+1;
                if(i)best=std::min(best,dp[at(i-1,j)]+gapCost);
                if(j)best=std::min(best,dp[at(i,j-1)]+gapCost);
                const int diagonal=i&&j?(bitAt(a,a0+i-1)==bitAt(b,b0+j-1)?0:substitution):0;
                if(i&&j)best=std::min(best,dp[at(i-1,j-1)]+diagonal);
                dp[at(i,j)]=best;
                if(i&&dp[at(i-1,j)]+1==best)count[at(i,j)]+=count[at(i-1,j)];
                if(j&&dp[at(i,j-1)]+1==best)count[at(i,j)]+=count[at(i,j-1)];
                if(i&&j&&dp[at(i-1,j-1)]+diagonal==best)
                    count[at(i,j)]+=count[at(i-1,j-1)];
            }
            return {dp[at(m,n)],count[at(m,n)],true};
        };
        for(int substitution=1;substitution<=2;++substitution)for(int gapCost=1;gapCost<=3;++gapCost){
            std::vector<Score> scores(scenarios);
            for(size_t mask=0;mask<scenarios;++mask){
                size_t cursorA=v.a0,cursorB=v.b0;
                Score result;
                for(size_t h=0;h<=hits.size();++h){
                    if(h<hits.size()&&!(mask&(size_t(1)<<h)))continue;
                    const size_t nextA=h==hits.size()?v.a1:hits[h].a;
                    const size_t nextB=h==hits.size()?v.b1:hits[h].b;
                    if(nextA<cursorA||nextB<cursorB){result.valid=false;break;}
                    const Score part=scoreSegment(cursorA,nextA,cursorB,nextB,substitution,gapCost);
                    result.cost+=part.cost;
                    result.paths=result.paths*part.paths;
                    if(h==hits.size())break;
                    for(size_t k=0;k<hits[h].len;++k)
                        if(bitAt(a,hits[h].a+k)!=bitAt(b,hits[h].b+k))result.valid=false;
                    if(!result.valid)break;
                    cursorA=hits[h].a+hits[h].len;
                    cursorB=hits[h].b+hits[h].len;
                }
                scores[mask]=result;
            }
            const auto& baseline=scores[0];
            std::printf("SPR_M271_PROFILE resource=%s interval=%zu substitution=%d insertion=%d deletion=%d unconstrainedCost=%d unconstrainedPaths=%s\n",
                name.c_str(),ix+1,substitution,gapCost,gapCost,baseline.cost,baseline.paths.str().c_str());
            for(size_t mask=1;mask<scenarios;++mask){
                const auto& result=scores[mask];
                const int penalty=result.cost-baseline.cost;
                const bool any=result.valid&&penalty==0;
                const bool all=any&&!(result.paths!=baseline.paths);
                std::printf("SPR_M271_ANCHOR resource=%s interval=%zu substitution=%d gapCost=%d mask=%zu constrainedCost=%d penalty=%d constrainedPaths=%s anyOptimal=%d allOptimal=%d valid=%d\n",
                    name.c_str(),ix+1,substitution,gapCost,mask,result.cost,penalty,result.paths.str().c_str(),
                    any?1:0,all?1:0,result.valid?1:0);
                if(!result.valid||penalty<0|| (all&& !any))return false;
            }
        }
    }
    return true;
}
}
int stafuSprM271GapCostSensitivity(const char* installed,const char* cd,const char*){
    const std::vector<std::vector<size_t>> picard={
        {5,1,2,3204,4489,3234,4526},{5,1,2,4967,5093,5004,5120},
        {5,1,2,9730,10499,9792,10603},{5,3,4,6342,6591,6464,6713}};
    const std::vector<std::vector<size_t>> data={
        {0,0,1,9973,10390,9966,10356},{0,0,1,10781,11976,10747,11952}};
    if(!load(installed,cd,"BRDGPICA.SPR",picard)||
       !load(installed,cd,"BRDGDATA.SPR",data))return 1;
    std::puts("SPR_M271_SUMMARY intervals=3 scenarios=8 resources=2");
    std::puts("SPR_M271_NOTE Substitution costs 1/2 and insertion/deletion costs 1/2/3; exact path counts and enforced-anchor cost penalties. No codec interpretation.");
    return 0;
}
