// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only candidate-target validation for opaque four-byte SPR image payloads.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_sequence_index.h"
#include "engines/stafu/spr_image_payload.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace {
struct Boundary {
    size_t offset,record,chunk;
    std::string kind;
};
struct Entry {
    size_t record,image,chunk,chunkOffset,imageOffset;
    uint32_t value,a,b,marker;
    std::string tag;
};
struct Key {
    std::string tag;
    uint32_t a,b;
    int64_t anchor;
    bool operator<(const Key& o) const {
        return std::tie(tag,a,b,anchor)<std::tie(o.tag,o.a,o.b,o.anchor);
    }
};
void classify(size_t target,const std::vector<Boundary>& boundaries,
              const std::vector<reluva::stafu::SprSequenceIndex::Record>& records,
              const char* label) {
    size_t exact=0;
    for(const auto& b:boundaries)if(b.offset==target) {
        if(exact<6)std::printf(" %s=%s:r%zu:c%zu",label,b.kind.c_str(),b.record,b.chunk);
        ++exact;
    }
    if(!exact) {
        size_t closest=static_cast<size_t>(-1);
        const Boundary* best=nullptr;
        for(const auto& b:boundaries) {
            const size_t distance=b.offset>target?b.offset-target:target-b.offset;
            if(distance<closest){closest=distance;best=&b;}
        }
        if(best)std::printf(" %s=nearest:%s:r%zu:c%zu:distance%zu",
                            label,best->kind.c_str(),best->record,best->chunk,closest);
    } else if(exact>6)std::printf(" %s_extra=%zu",label,exact-6);
    for(size_t r=0;r<records.size();++r)
        if(target>=records[r].start&&target<records[r].end) {
            std::printf(" %s_inRecord=%zu",label,r);
            break;
        }
}
}

int stafuSprOffsetTargets(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-offset-targets: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-offset-targets: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-offset-targets: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        const auto& records=index.records();
        std::vector<Boundary> boundaries;
        std::vector<Entry> entries;
        for(size_t r=0;r<records.size();++r) {
            boundaries.push_back({records[r].start,r,0,"recordStart"});
            boundaries.push_back({records[r].end,r,0,"recordEnd"});
            size_t ordinal=0;
            for(size_t c=0;c<records[r].events.size();++c) {
                const auto& e=records[r].events[c];
                boundaries.push_back({e.chunkOffset,r,c,"chunkStart"});
                boundaries.push_back({e.chunkOffset+e.chunkSize,r,c,"chunkEnd"});
                boundaries.push_back({e.payloadOffset,r,c,"payloadStart"});
                if(e.hasImageHeader)boundaries.push_back({e.imageDataOffset,r,c,"imageStart"});
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,view)&&
                   view.size==4&&view.marker==0x00030000) {
                    const auto* p=view.data;
                    uint32_t value=uint32_t(p[0])|(uint32_t(p[1])<<8)|
                                   (uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
                    entries.push_back({r,ordinal,c,e.chunkOffset,e.imageDataOffset,
                                       value,view.headerA,view.headerB,view.marker,e.tag});
                }
                ++ordinal;
            }
        }
        std::map<Key,std::vector<size_t>> chunkAnchors,imageAnchors;
        size_t absoluteExact=0,recordExact=0;
        std::printf("SPR_TARGET_RESOURCE resource=%s fileSize=%zu records=%zu values=%zu\n",
                    name.c_str(),bytes.size(),records.size(),entries.size());
        for(const auto& e:entries) {
            const int64_t chunkAnchor=static_cast<int64_t>(e.chunkOffset)-e.value;
            const int64_t imageAnchor=static_cast<int64_t>(e.imageOffset)-e.value;
            chunkAnchors[{e.tag,e.a,e.b,chunkAnchor}].push_back(e.image);
            imageAnchors[{e.tag,e.a,e.b,imageAnchor}].push_back(e.image);
            const size_t recordTarget=records[e.record].start+static_cast<size_t>(e.value);
            auto exact=[&](size_t target) {
                for(const auto& b:boundaries)if(b.offset==target)return true;
                return false;
            };
            if(e.value<bytes.size()&&exact(e.value))++absoluteExact;
            if(e.value<=records[e.record].end-records[e.record].start&&exact(recordTarget))
                ++recordExact;
            std::printf("  SPR_TARGET record=%zu image=%zu chunk=%zu tag=%s dims=%u/%u value=%u chunk=0x%zx imageData=0x%zx chunkMinusValue=%lld imageMinusValue=%lld",
                        e.record,e.image,e.chunk,e.tag.c_str(),e.a,e.b,e.value,
                        e.chunkOffset,e.imageOffset,
                        static_cast<long long>(chunkAnchor),
                        static_cast<long long>(imageAnchor));
            if(e.value<bytes.size())classify(e.value,boundaries,records,"absolute");
            else std::printf(" absolute=outOfFile");
            if(e.value<=records[e.record].end-records[e.record].start)
                classify(recordTarget,boundaries,records,"recordRelative");
            else std::printf(" recordRelative=outsideSourceRecord");
            std::printf("\n");
        }
        auto printAnchors=[&](const char* label,const std::map<Key,std::vector<size_t>>& groups) {
            size_t repeated=0;
            for(const auto& [key,images]:groups) {
                if(images.size()<2)continue;
                ++repeated;
                std::printf("  SPR_TARGET_ANCHOR basis=%s tag=%s dims=%u/%u anchor=%lld count=%zu images=",
                            label,key.tag.c_str(),key.a,key.b,
                            static_cast<long long>(key.anchor),images.size());
                for(size_t i=0;i<images.size();++i)
                    std::printf("%s%zu",i?",":"",images[i]);
                if(key.anchor>=0&&static_cast<uint64_t>(key.anchor)<bytes.size())
                    classify(static_cast<size_t>(key.anchor),boundaries,records,"anchor");
                else std::printf(" anchor=outsideFile");
                std::printf("\n");
            }
            return repeated;
        };
        const size_t repeatedChunk=printAnchors("chunkStart",chunkAnchors);
        const size_t repeatedImage=printAnchors("imageDataStart",imageAnchors);
        std::printf("SPR_TARGET_SUMMARY resource=%s sources=%zu absoluteBoundaryHits=%zu recordBoundaryHits=%zu repeatedChunkAnchors=%zu repeatedImageAnchors=%zu\n",
                    name.c_str(),entries.size(),absoluteExact,recordExact,
                    repeatedChunk,repeatedImage);
    }
    std::puts("SPR_TARGET_NOTE exact matches and stable anchors are structural evidence, not proof of pointer semantics.");
    return failures?1:0;
}


// Milestone 224: resolve repeated source-offset anchors to observed image chunks.
// The relation imageDataOffset - value = anchor is a hypothesis, not a pointer.
int stafuSprReferenceResolve(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-reference-resolve: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-reference-resolve: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-reference-resolve: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Image {
            size_t record,image,chunk,offset,size;
            std::string tag;
            uint32_t marker,a,b;
        };
        struct Source {
            size_t record,image,chunk,offset;
            uint32_t value;
        };
        using GroupKey=std::tuple<std::string,uint32_t,uint32_t,int64_t>;
        std::vector<Image> images;
        std::map<GroupKey,std::vector<Source>> groups;
        const auto& records=index.records();
        for(size_t r=0;r<records.size();++r) {
            size_t ordinal=0;
            for(size_t c=0;c<records[r].events.size();++c) {
                const auto& e=records[r].events[c];
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,e,view)) {
                    ++ordinal;continue;
                }
                images.push_back({r,ordinal,c,view.offset,view.size,e.tag,
                                  view.marker,view.headerA,view.headerB});
                if(view.size==4&&view.marker==0x00030000) {
                    const auto* p=view.data;
                    const uint32_t value=uint32_t(p[0])|(uint32_t(p[1])<<8)|
                                         (uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
                    const int64_t anchor=static_cast<int64_t>(view.offset)-
                                         static_cast<int64_t>(value);
                    groups[{e.tag,view.headerA,view.headerB,anchor}].push_back(
                        {r,ordinal,c,view.offset,value});
                }
                ++ordinal;
            }
        }
        size_t repeated=0,exact=0,compatible=0,backward=0,forward=0;
        std::printf("SPR_RESOLVE_RESOURCE resource=%s records=%zu images=%zu groups=%zu\n",
                    name.c_str(),records.size(),images.size(),groups.size());
        for(const auto& [key,sources]:groups) {
            if(sources.size()<2)continue;
            ++repeated;
            const auto& [tag,a,b,anchor]=key;
            std::vector<const Image*> targets;
            for(const auto& im:images)
                if(anchor>=0&&static_cast<uint64_t>(anchor)==im.offset)
                    targets.push_back(&im);
            if(!targets.empty())++exact;
            bool match=false;
            for(const auto* im:targets)
                if(im->tag==tag&&im->a==a&&im->b==b)match=true;
            if(match)++compatible;
            std::printf("  SPR_RESOLVE_GROUP tag=%s dims=%u/%u anchor=%lld count=%zu exactImageStarts=%zu compatibleTarget=%s\n",
                        tag.c_str(),a,b,static_cast<long long>(anchor),
                        sources.size(),targets.size(),match?"yes":"no");
            for(const auto* im:targets) {
                std::printf("    TARGET record=%zu image=%zu chunk=%zu tag=%s dims=%u/%u marker=0x%08x imageDataSize=%zu headerMatch=%s\n",
                            im->record,im->image,im->chunk,im->tag.c_str(),im->a,im->b,
                            im->marker,im->size,
                            im->tag==tag&&im->a==a&&im->b==b?"yes":"no");
            }
            for(const auto& src:sources) {
                const char* direction="unknown";
                if(!targets.empty()) {
                    if(src.offset>targets.front()->offset){direction="after";++forward;}
                    else if(src.offset<targets.front()->offset){direction="before";++backward;}
                    else direction="same";
                }
                // Value is the distance from the proposed anchor to the source
                // image-data offset, not necessarily a target-file pointer.
                std::printf("    SOURCE record=%zu image=%zu chunk=%zu imageData=0x%zx value=%u relativeToAnchor=%s",
                            src.record,src.image,src.chunk,src.offset,src.value,direction);
                if(!targets.empty()) {
                    const auto* im=targets.front();
                    const bool inside=src.offset>=im->offset&&
                                      src.offset-im->offset<im->size;
                    std::printf(" withinTargetPayload=%s",inside?"yes":"no");
                }
                std::printf("\n");
            }
        }
        std::printf("SPR_RESOLVE_SUMMARY resource=%s repeatedGroups=%zu exactAnchorGroups=%zu compatibleAnchorGroups=%zu sourcesBefore=%zu sourcesAfter=%zu\n",
                    name.c_str(),repeated,exact,compatible,backward,forward);
    }
    std::puts("SPR_RESOLVE_NOTE imageDataOffset-value anchors are observational; matching metadata or offsets does not prove image reuse, decoding, or pointer semantics.");
    return failures?1:0;
}


// Milestone 225: validate every candidate, including singleton references.
// No image bytes are decoded or modified.
int stafuSprReferenceValidate(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-reference-validate: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-reference-validate: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-reference-validate: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Node {
            size_t record,image,chunk,offset,size;
            uint32_t a,b,marker,value;
            std::string tag;
            bool candidate;
        };
        std::vector<Node> nodes;
        std::map<size_t,std::vector<size_t>> byOffset;
        const auto& records=index.records();
        for(size_t r=0;r<records.size();++r) {
            size_t ordinal=0;
            for(size_t c=0;c<records[r].events.size();++c) {
                const auto& e=records[r].events[c];
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,view)) {
                    const bool candidate=view.size==4&&view.marker==0x00030000;
                    uint32_t value=0;
                    if(candidate) {
                        const auto* p=view.data;
                        value=uint32_t(p[0])|(uint32_t(p[1])<<8)|
                              (uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
                    }
                    byOffset[view.offset].push_back(nodes.size());
                    nodes.push_back({r,ordinal,c,view.offset,view.size,
                                     view.headerA,view.headerB,view.marker,value,
                                     e.tag,candidate});
                }
                ++ordinal;
            }
        }
        size_t candidates=0,exact=0,compatible=0,mismatch=0,missing=0,
               outOfRange=0,forward=0,backward=0,ambiguous=0,chains=0,cycles=0;
        std::vector<int> edge(nodes.size(),-1);
        std::printf("SPR_VALIDATE_RESOURCE resource=%s records=%zu images=%zu\n",
                    name.c_str(),records.size(),nodes.size());
        for(size_t i=0;i<nodes.size();++i) {
            const auto& src=nodes[i];
            if(!src.candidate)continue;
            ++candidates;
            if(src.value>src.offset) {
                ++outOfRange;
                std::printf("  SPR_VALIDATE_SOURCE record=%zu image=%zu offset=0x%zx value=%u status=underflow\n",
                            src.record,src.image,src.offset,src.value);
                continue;
            }
            const size_t targetOffset=src.offset-src.value;
            const auto found=byOffset.find(targetOffset);
            if(found==byOffset.end()) {
                ++missing;
                std::printf("  SPR_VALIDATE_SOURCE record=%zu image=%zu offset=0x%zx value=%u target=0x%zx status=missing\n",
                            src.record,src.image,src.offset,src.value,targetOffset);
                continue;
            }
            ++exact;
            if(found->second.size()>1)++ambiguous;
            int selected=-1;
            for(const size_t j:found->second) {
                const auto& dst=nodes[j];
                if(src.tag==dst.tag&&src.a==dst.a&&src.b==dst.b) {
                    selected=static_cast<int>(j);
                    break;
                }
            }
            if(selected<0) {
                ++mismatch;
                selected=static_cast<int>(found->second.front());
            } else ++compatible;
            edge[i]=selected;
            const auto& dst=nodes[static_cast<size_t>(selected)];
            if(dst.offset<src.offset)++backward;
            else if(dst.offset>src.offset)++forward;
            std::printf("  SPR_VALIDATE_SOURCE record=%zu image=%zu offset=0x%zx value=%u target=0x%zx status=%s targetRecord=%zu targetImage=%zu targetTag=%s targetDims=%u/%u targetMarker=0x%08x targetSize=%zu targetCandidate=%s matches=%zu\n",
                        src.record,src.image,src.offset,src.value,targetOffset,
                        src.tag==dst.tag&&src.a==dst.a&&src.b==dst.b?"compatible":"metadataMismatch",
                        dst.record,dst.image,dst.tag.c_str(),dst.a,dst.b,
                        dst.marker,dst.size,dst.candidate?"yes":"no",found->second.size());
        }
        // Follow edges without recursion, bounding traversal by the number of nodes.
        // Count a chain when a candidate points to another candidate.
        for(size_t i=0;i<nodes.size();++i) {
            if(!nodes[i].candidate||edge[i]<0)continue;
            if(nodes[static_cast<size_t>(edge[i])].candidate)++chains;
            std::vector<unsigned char> seen(nodes.size(),0);
            int current=static_cast<int>(i);
            for(size_t step=0;current>=0&&step<=nodes.size();++step) {
                const size_t at=static_cast<size_t>(current);
                if(seen[at]) {
                    ++cycles;
                    std::printf("  SPR_VALIDATE_CYCLE sourceRecord=%zu sourceImage=%zu atRecord=%zu atImage=%zu\n",
                                nodes[i].record,nodes[i].image,nodes[at].record,nodes[at].image);
                    break;
                }
                seen[at]=1;
                current=edge[at];
            }
        }
        std::printf("SPR_VALIDATE_SUMMARY resource=%s candidates=%zu exact=%zu compatible=%zu metadataMismatch=%zu missing=%zu underflow=%zu ambiguous=%zu backward=%zu forward=%zu directChains=%zu cycleSources=%zu\n",
                    name.c_str(),candidates,exact,compatible,mismatch,missing,
                    outOfRange,ambiguous,backward,forward,chains,cycles);
    }
    std::puts("SPR_VALIDATE_NOTE candidate offset interpretation only; no sprite reconstruction or decoding performed.");
    return failures?1:0;
}


// Milestone 228: observational payload-size/prefix and cross-tag comparisons.
// This deliberately does not infer pixel decoding semantics.
int stafuSprPayloadStructureCompare(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-payload-structure: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-payload-structure: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-payload-structure: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Image {
            size_t record,ordinal,offset,size;
            uint32_t a,b,marker,value;
            std::string tag;
            bool reference;
        };
        std::vector<Image> images;
        std::map<size_t,size_t> offsets;
        std::map<size_t,size_t> sizes;
        const auto& records=index.records();
        for(size_t r=0;r<records.size();++r) {
            size_t ordinal=0;
            for(const auto& event:records[r].events) {
                if(event.tag!="PMOC"&&event.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,event,view)) {
                    const bool reference=view.size==4&&view.marker==0x00030000;
                    uint32_t value=0;
                    if(reference) {
                        const auto* p=view.data;
                        value=uint32_t(p[0])|(uint32_t(p[1])<<8)|
                              (uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
                    }
                    offsets.emplace(view.offset,images.size());
                    images.push_back({r,ordinal,view.offset,view.size,view.headerA,
                                      view.headerB,view.marker,value,event.tag,reference});
                    if(!reference)++sizes[view.size];
                }
                ++ordinal;
            }
        }
        size_t references=0,exact=0,tagOnly=0,dimensionMismatch=0,shortRefs=0;
        std::printf("SPR_STRUCTURE_RESOURCE resource=%s records=%zu images=%zu\n",
                    name.c_str(),records.size(),images.size());
        for(const auto& [size,count]:sizes)
            if(size<=16)
                std::printf("SPR_STRUCTURE_SHORT_SIZE resource=%s size=%zu count=%zu\n",
                            name.c_str(),size,count);
        for(const auto& im:images) {
            if(im.reference)continue;
            if(im.size>16)continue;
            std::printf("SPR_STRUCTURE_SHORT resource=%s record=%zu image=%zu tag=%s dims=%u/%u marker=0x%08x offset=0x%zx size=%zu prefix=",
                        name.c_str(),im.record,im.ordinal,im.tag.c_str(),im.a,im.b,
                        im.marker,im.offset,im.size);
            for(size_t b=0;b<im.size;++b)std::printf("%02x",unsigned(bytes[im.offset+b]));
            std::printf("\n");
        }
        for(const auto& src:images) {
            if(!src.reference)continue;
            ++references;
            if(src.value>src.offset)continue;
            const auto it=offsets.find(src.offset-src.value);
            if(it==offsets.end())continue;
            ++exact;
            const auto& dst=images[it->second];
            const bool dims=src.a==dst.a&&src.b==dst.b;
            const bool tag=src.tag==dst.tag;
            if(dims&&!tag)++tagOnly;
            if(!dims)++dimensionMismatch;
            if(dst.size<=16)++shortRefs;
            if((dims&&!tag)||dst.size<=16) {
                std::printf("SPR_STRUCTURE_REFERENCE resource=%s sourceRecord=%zu sourceImage=%zu sourceTag=%s sourceDims=%u/%u sourceMarker=0x%08x sourceOffset=0x%zx targetRecord=%zu targetImage=%zu targetTag=%s targetDims=%u/%u targetMarker=0x%08x targetOffset=0x%zx targetSize=%zu classification=%s\n",
                            name.c_str(),src.record,src.ordinal,src.tag.c_str(),src.a,src.b,
                            src.marker,src.offset,dst.record,dst.ordinal,dst.tag.c_str(),
                            dst.a,dst.b,dst.marker,dst.offset,dst.size,
                            !dims?"dimensionMismatch":!tag?"tagOnlyMismatch":"shortTarget");
            }
        }
        // Compare each short payload to other non-reference payloads of the same
        // tag and dimensions. Prefix equality is not a decoding assertion.
        for(const auto& src:images) {
            if(src.reference||src.size>16)continue;
            size_t peers=0,prefixMatches=0;
            for(const auto& dst:images) {
                if(&src==&dst||dst.reference||src.tag!=dst.tag||
                   src.a!=dst.a||src.b!=dst.b)continue;
                ++peers;
                const size_t n=std::min<size_t>(src.size,dst.size);
                if(n&&std::equal(bytes.begin()+src.offset,
                                 bytes.begin()+src.offset+n,bytes.begin()+dst.offset))
                    ++prefixMatches;
            }
            std::printf("SPR_STRUCTURE_PEERS resource=%s offset=0x%zx dims=%u/%u tag=%s sameMetadataPeers=%zu prefixMatches=%zu\n",
                        name.c_str(),src.offset,src.a,src.b,src.tag.c_str(),peers,prefixMatches);
        }
        std::printf("SPR_STRUCTURE_SUMMARY resource=%s references=%zu exactTargets=%zu tagOnlyMismatch=%zu dimensionMismatch=%zu shortTargetReferences=%zu\n",
                    name.c_str(),references,exact,tagOnly,dimensionMismatch,shortRefs);
    }
    std::puts("SPR_STRUCTURE_NOTE read-only structural comparison; prefix matching and marker values do not establish image encoding or renderability.");
    return failures?1:0;
}


// Milestone 229: read-only direct-image payload encoding survey.
// A marker is an observed field, not an asserted codec identifier.
int stafuSprEncodingSurvey(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-encoding-survey: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-encoding-survey: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-encoding-survey: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Stats {
            size_t count=0,minSize=static_cast<size_t>(-1),maxSize=0,total=0;
            size_t shorterThanArea=0,shortUpTo16=0,zero=0;
        };
        std::map<std::pair<std::string,uint32_t>,Stats> stats;
        size_t direct=0,references=0,invalid=0;
        std::printf("SPR_ENCODING_RESOURCE resource=%s records=%zu\n",
                    name.c_str(),index.records().size());
        for(size_t ri=0;ri<index.records().size();++ri) {
            size_t ordinal=0;
            for(const auto& e:index.records()[ri].events) {
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View view;
                if(!reluva::stafu::SprImagePayloadAccessor::get(bytes,e,view)) {
                    ++invalid;++ordinal;continue;
                }
                if(view.marker==0x00030000&&view.size==4) {
                    ++references;++ordinal;continue;
                }
                ++direct;
                auto& s=stats[{e.tag,view.marker}];
                ++s.count;
                s.minSize=std::min(s.minSize,view.size);
                s.maxSize=std::max(s.maxSize,view.size);
                s.total+=view.size;
                const uint64_t area=uint64_t(view.headerA)*uint64_t(view.headerB);
                if(view.size<area)++s.shorterThanArea;
                if(view.size<=16)++s.shortUpTo16;
                if(!view.size)++s.zero;
                std::printf("SPR_ENCODING_IMAGE resource=%s record=%zu image=%zu tag=%s dims=%u/%u area=%llu marker=0x%08x offset=0x%zx size=%zu bytesPerArea=",
                            name.c_str(),ri,ordinal,e.tag.c_str(),view.headerA,view.headerB,
                            static_cast<unsigned long long>(area),view.marker,view.offset,view.size);
                if(area)std::printf("%.6f",double(view.size)/double(area));
                else std::printf("undefined");
                std::printf(" prefix=");
                for(size_t i=0;i<std::min<size_t>(view.size,12);++i)
                    std::printf("%02x",unsigned(view.data[i]));
                std::printf("\n");
                ++ordinal;
            }
        }
        for(const auto& [key,s]:stats)
            std::printf("SPR_ENCODING_GROUP resource=%s tag=%s marker=0x%08x count=%zu minSize=%zu maxSize=%zu totalBytes=%zu belowArea=%zu shortUpTo16=%zu zero=%zu\n",
                        name.c_str(),key.first.c_str(),key.second,s.count,
                        s.minSize,s.maxSize,s.total,s.shorterThanArea,s.shortUpTo16,s.zero);
        std::printf("SPR_ENCODING_SUMMARY resource=%s direct=%zu references=%zu invalid=%zu markerGroups=%zu\n",
                    name.c_str(),direct,references,invalid,stats.size());
    }
    std::puts("SPR_ENCODING_NOTE observational byte lengths and prefixes only; no decompression, pixel interpretation, or codec identification.");
    return failures?1:0;
}
