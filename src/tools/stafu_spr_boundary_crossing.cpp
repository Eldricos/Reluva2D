// SPDX-License-Identifier: GPL-3.0-or-later
// M259: verify boundary-crossing signatures in both SPR payloads.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
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
          const std::vector<std::vector<size_t>>& gaps,std::vector<Boundary>& out){
    reluva::stafu::ResourceManager manager;std::string error;
    if(!manager.open(installed,cd,error)){std::fprintf(stderr,"SPR_M259_ERROR %s\n",error.c_str());return false;}
    std::vector<uint8_t> bytes;
    if(!manager.read(name,bytes,error)){std::fprintf(stderr,"SPR_M259_ERROR %s\n",error.c_str());return false;}
    reluva::stafu::SprSequenceIndex index;
    if(!index.build(bytes,error)){std::fprintf(stderr,"SPR_M259_ERROR %s\n",error.c_str());return false;}
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
    for(size_t gi=0;gi<gaps.size();++gi){
        const auto& g=gaps[gi];
        const Sample *sa=nullptr,*sb=nullptr;
        for(const auto& x:samples){
            if(x.record==g[0]&&x.image==g[1])sa=&x;
            if(x.record==g[0]&&x.image==g[2])sb=&x;
        }
        if(!sa||!sb||g[4]>sa->size*8||g[6]>sb->size*8){
            std::printf("SPR_M259_ERROR resource=%s gap=%zu reason=invalid-payload-or-range\n",name.c_str(),gi+1);
            return false;
        }
        const uint8_t* a=bytes.data()+sa->offset;
        const uint8_t* b=bytes.data()+sb->offset;
        const size_t na=g[4]-g[3],nb=g[6]-g[5];
        auto evaluate=[&](int shift){
            const size_t aa=shift<0?size_t(-shift):0;
            const size_t bb=shift>0?size_t(shift):0;
            Alignment v{shift,aa,bb,0,0};
            if(aa>=na||bb>=nb)return v;
            v.compared=std::min(na-aa,nb-bb);
            for(size_t i=0;i<v.compared;++i)
                v.equal+=bitAt(a,g[3]+aa+i)==bitAt(b,g[5]+bb+i);
            return v;
        };
        Alignment best=evaluate(0);
        for(int shift=-32;shift<=32;++shift){
            Alignment candidate=evaluate(shift);
            if(better(candidate,best))best=candidate;
        }
        Run longest{0,0,true};
        Run current{0,0,true};
        for(size_t i=0;i<best.compared;++i){
            const bool equal=bitAt(a,g[3]+best.startA+i)==bitAt(b,g[5]+best.startB+i);
            if(equal){
                if(!current.length)current.start=i;
                ++current.length;
                if(current.length>longest.length)longest=current;
            }else current.length=0;
        }
        if(longest.length<32){std::printf("SPR_M259_ERROR resource=%s gap=%zu reason=no-long-run\n",name.c_str(),gi+1);return false;}
        for(int side=0;side<2;++side){
            const size_t boundary=side==0?longest.start:longest.start+longest.length;
            const size_t left=std::min(size_t(64),boundary);
            const size_t right=std::min(size_t(64),best.compared-boundary);
            Boundary item{name,gi+1,g[0],g[1],g[2],best.shift,boundary,longest.length,side==0,"",""};
            for(size_t j=boundary-left;j<boundary+right;++j){
                item.a+=bitAt(a,g[3]+best.startA+j)?'1':'0';
                item.b+=bitAt(b,g[5]+best.startB+j)?'1':'0';
            }
            // Store boundary-centered context with left and right extents separately.
            // A sentinel separates the flanks; comparisons never cross the sentinel.
            item.a.insert(left,"|");item.b.insert(left,"|");
            out.push_back(item);
            std::printf("SPR_M259_BOUNDARY resource=%s gap=%zu side=%s shift=%d run=%zu left=%zu right=%zu\n",
                name.c_str(),gi+1,side==0?"start":"end",best.shift,longest.length,left,right);
        }
    }
    return true;
}
std::string flank(const std::string& s,bool left){
    const size_t p=s.find('|');
    return left?s.substr(0,p):s.substr(p+1);
}
void compare(const Boundary& x,const Boundary& y,size_t xi,size_t yi){
    // Compare 16 bits on each side of each boundary. The displacement
    // moves the second boundary window and is applied identically to A and B.
    const size_t xmid=x.a.find('|'),ymid=y.a.find('|');
    for(int shift=-8;shift<=8;++shift){
        const int xleft=int(xmid)-16;
        const int yleft=int(ymid)-16+shift;
        if(xleft<0||yleft<0||
           xleft+32>int(x.a.size())-1||yleft+32>int(y.a.size())-1)continue;
        auto bit=[&](const std::string& str,size_t middle,int relative){
            return str[size_t(int(middle)+relative+(relative>=0?1:0))];
        };
        size_t matchesA=0,matchesB=0,leftA=0,leftB=0,rightA=0,rightB=0;
        for(int j=-16;j<16;++j){
            const char xa=bit(x.a,xmid,j),ya=bit(y.a,ymid,j+shift);
            const char xb=bit(x.b,xmid,j),yb=bit(y.b,ymid,j+shift);
            const bool ma=xa==ya,mb=xb==yb;
            matchesA+=ma;matchesB+=mb;
            if(j<0){leftA+=ma;leftB+=mb;}
            else{rightA+=ma;rightB+=mb;}
        }
        if(matchesA>=26&&matchesB>=26){
            std::printf("SPR_M259_CANDIDATE first=%zu second=%zu shift=%d matchA=%zu matchB=%zu leftA=%zu leftB=%zu rightA=%zu rightB=%zu bothExact=%d\n",
                xi,yi,shift,matchesA,matchesB,leftA,leftB,rightA,rightB,
                matchesA==32&&matchesB==32?1:0);
        }
    }
}

}
}
int stafuSprM259BoundaryCrossing(const char* installed,const char* cd,const char*){
    std::vector<Boundary> all;
    const std::vector<std::vector<size_t>> picard={
        {5,1,2,3204,4489,3234,4526},{5,1,2,4967,5093,5004,5120},
        {5,1,2,9730,10499,9792,10603},{5,3,4,6342,6591,6464,6713}};
    const std::vector<std::vector<size_t>> data={
        {0,0,1,9973,10390,9966,10356},{0,0,1,10781,11976,10747,11952}};
    if(!load(installed,cd,"BRDGPICA.SPR",picard,all)||
       !load(installed,cd,"BRDGDATA.SPR",data,all))return 1;
    size_t pairs=0;
    for(size_t i=0;i<all.size();++i)
        for(size_t j=i+1;j<all.size();++j){
            if(all[i].gap==all[j].gap&&all[i].resource==all[j].resource)continue;
            ++pairs;compare(all[i],all[j],i+1,j+1);
        }
    std::printf("SPR_M259_SUMMARY boundaries=%zu crossGapPairs=%zu\n",all.size(),pairs);
    std::puts("SPR_M259_NOTE MSB-first, observational longest-run boundaries. Boundary-centered 16+16 windows with local shift +/-8. Candidates require >=26/32 matches in both A and B. No codec semantics inferred.");
    return all.size()==12?0:1;
}
