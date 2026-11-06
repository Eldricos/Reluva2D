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


// Milestone 230: bounded, read-only byte-pattern analysis; no bitstream decoding.
int stafuSprBitPatternSurvey(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-bit-patterns: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-bit-patterns: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-bit-patterns: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Sample {
            size_t record,ordinal,offset,size;
            uint32_t a,b,marker;
            std::string tag;
        };
        std::vector<Sample> samples;
        std::map<uint32_t,std::vector<size_t>> byMarker;
        size_t references=0;
        for(size_t ri=0;ri<index.records().size();++ri) {
            size_t ordinal=0;
            for(const auto& e:index.records()[ri].events) {
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)) {
                    if(v.marker==0x00030000&&v.size==4)++references;
                    else {
                        byMarker[v.marker].push_back(samples.size());
                        samples.push_back({ri,ordinal,v.offset,v.size,v.headerA,v.headerB,v.marker,e.tag});
                    }
                }
                ++ordinal;
            }
        }
        std::printf("SPR_BITS_RESOURCE resource=%s direct=%zu references=%zu\n",
                    name.c_str(),samples.size(),references);
        for(const auto& [marker,indices]:byMarker) {
            size_t histogram[256]={};
            size_t total=0,adjacentEqual=0,tripletMatches=0;
            std::map<uint32_t,size_t> triples;
            for(size_t idx:indices) {
                const auto& s=samples[idx];
                for(size_t j=0;j<s.size;++j) {
                    ++histogram[bytes[s.offset+j]];++total;
                    if(j&&bytes[s.offset+j]==bytes[s.offset+j-1])++adjacentEqual;
                    if(j>=2) {
                        const uint32_t k=(uint32_t(bytes[s.offset+j-2])<<16)|
                                         (uint32_t(bytes[s.offset+j-1])<<8)|bytes[s.offset+j];
                        ++triples[k];
                    }
                }
            }
            std::vector<std::pair<size_t,unsigned>> ranked;
            for(unsigned b=0;b<256;++b)if(histogram[b])ranked.push_back({histogram[b],b});
            std::sort(ranked.begin(),ranked.end(),[](const auto& a,const auto& b){
                return a.first!=b.first?a.first>b.first:a.second<b.second;
            });
            std::printf("SPR_BITS_MARKER resource=%s marker=0x%08x images=%zu bytes=%zu adjacentEqual=%zu distinctBytes=%zu topBytes=",
                        name.c_str(),marker,indices.size(),total,adjacentEqual,ranked.size());
            for(size_t j=0;j<std::min<size_t>(8,ranked.size());++j)
                std::printf("%s%02x:%zu",j?",":"",ranked[j].second,ranked[j].first);
            std::printf("\n");
            std::vector<std::pair<size_t,uint32_t>> rankedTriples;
            for(const auto& [key,count]:triples)if(count>=2)rankedTriples.push_back({count,key});
            std::sort(rankedTriples.begin(),rankedTriples.end(),[](const auto& a,const auto& b){
                return a.first!=b.first?a.first>b.first:a.second<b.second;
            });
            std::printf("SPR_BITS_TRIPLES resource=%s marker=0x%08x repeatedDistinct=%zu top=",
                        name.c_str(),marker,rankedTriples.size());
            for(size_t j=0;j<std::min<size_t>(8,rankedTriples.size());++j)
                std::printf("%s%06x:%zu",j?",":"",rankedTriples[j].second,rankedTriples[j].first);
            std::printf("\n");
        }
        size_t pairs=0,identicalPrefix12=0;
        for(size_t i=1;i<samples.size();++i) {
            const auto& a=samples[i-1];
            const auto& b=samples[i];
            if(a.record!=b.record||a.tag!=b.tag||a.marker!=b.marker||
               a.a!=b.a||a.b!=b.b)continue;
            ++pairs;
            const size_t limit=std::min(a.size,b.size);
            size_t prefix=0;
            while(prefix<limit&&bytes[a.offset+prefix]==bytes[b.offset+prefix])++prefix;
            if(prefix>=12)++identicalPrefix12;
            std::printf("SPR_BITS_NEIGHBOR resource=%s record=%zu imageA=%zu imageB=%zu tag=%s marker=0x%08x dims=%u/%u sizeA=%zu sizeB=%zu commonPrefix=%zu\n",
                        name.c_str(),a.record,a.ordinal,b.ordinal,a.tag.c_str(),a.marker,
                        a.a,a.b,a.size,b.size,prefix);
        }
        // Exact occurrence of Picard's three-byte payload, including overlapping
        // matches, within direct-image data only (not resource headers).
        size_t needleImages=0,needleOccurrences=0;
        for(const auto& s:samples) {
            size_t count=0;
            for(size_t j=0;j+3<=s.size;++j)
                if(bytes[s.offset+j]==0x3f&&bytes[s.offset+j+1]==0xca&&
                   bytes[s.offset+j+2]==0x30)++count;
            if(count) {
                ++needleImages;needleOccurrences+=count;
                std::printf("SPR_BITS_NEEDLE resource=%s record=%zu image=%zu tag=%s marker=0x%08x size=%zu matches=%zu\n",
                            name.c_str(),s.record,s.ordinal,s.tag.c_str(),s.marker,s.size,count);
            }
        }
        std::printf("SPR_BITS_SUMMARY resource=%s direct=%zu neighborPairs=%zu prefixAtLeast12=%zu needleImages=%zu needleOccurrences=%zu\n",
                    name.c_str(),samples.size(),pairs,identicalPrefix12,needleImages,needleOccurrences);
    }
    std::puts("SPR_BITS_NOTE byte histograms, 3-byte sequences and shared prefixes are descriptive only; no codec inference or decoding.");
    return failures?1:0;
}


// Milestone 231: observational bit alignment of neighboring direct SPR payloads.
// Bit order is explicitly MSB-first for this diagnostic, not a codec assertion.
int stafuSprBitAlignment(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-bit-alignment: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-bit-alignment: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-bit-alignment: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Sample {
            size_t record,ordinal,offset,size;
            uint32_t a,b,marker;
            std::string tag;
        };
        std::vector<Sample> samples;
        for(size_t ri=0;ri<index.records().size();++ri) {
            size_t ordinal=0;
            for(const auto& e:index.records()[ri].events) {
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
                   !(v.marker==0x00030000&&v.size==4))
                    samples.push_back({ri,ordinal,v.offset,v.size,v.headerA,v.headerB,v.marker,e.tag});
                ++ordinal;
            }
        }
        auto bit=[&bytes](const Sample& s,size_t pos)->unsigned {
            return (bytes[s.offset+pos/8]>>(7-(pos%8)))&1u;
        };
        size_t pairs=0,shiftWins=0,needleAligned=0,needleUnaligned=0;
        for(size_t i=1;i<samples.size();++i) {
            const auto& a=samples[i-1];
            const auto& b=samples[i];
            if(a.record!=b.record||a.tag!=b.tag||a.marker!=b.marker||
               a.a!=b.a||a.b!=b.b)continue;
            ++pairs;
            size_t bestShift=0,bestBits=0,unshiftedBits=0;
            // Compare the beginning of A to the beginning of B at shifts 0..7.
            // Cap comparisons at 2048 bits to bound diagnostic work.
            for(size_t shift=0;shift<8;++shift) {
                const size_t maxBits=std::min<size_t>(
                    2048,std::min(a.size*8,b.size*8>shift?b.size*8-shift:0));
                size_t matched=0;
                while(matched<maxBits&&bit(a,matched)==bit(b,matched+shift))++matched;
                if(shift==0)unshiftedBits=matched;
                if(matched>bestBits){bestBits=matched;bestShift=shift;}
            }
            if(bestShift)++shiftWins;
            std::printf("SPR_ALIGN_PAIR resource=%s record=%zu imageA=%zu imageB=%zu tag=%s marker=0x%08x dims=%u/%u sizeA=%zu sizeB=%zu prefixBits0=%zu bestShiftBits=%zu bestPrefixBits=%zu\n",
                        name.c_str(),a.record,a.ordinal,b.ordinal,a.tag.c_str(),a.marker,
                        a.a,a.b,a.size,b.size,unshiftedBits,bestShift,bestBits);
        }
        // Search for the exact 24-bit pattern 3F CA 30 at all bit positions,
        // including positions that are not byte-aligned. MSB-first only.
        for(const auto& s:samples) {
            const unsigned needle[3]={0x3f,0xca,0x30};
            size_t aligned=0,unaligned=0;
            for(size_t start=0;start+24<=s.size*8;++start) {
                bool match=true;
                for(size_t k=0;k<24;++k)
                    if(bit(s,start+k)!=((needle[k/8]>>(7-(k%8)))&1u)){
                        match=false;break;
                    }
                if(match) {
                    if(start%8==0)++aligned;
                    else ++unaligned;
                }
            }
            if(aligned||unaligned) {
                needleAligned+=aligned;needleUnaligned+=unaligned;
                std::printf("SPR_ALIGN_NEEDLE resource=%s record=%zu image=%zu tag=%s marker=0x%08x size=%zu aligned=%zu unaligned=%zu\n",
                            name.c_str(),s.record,s.ordinal,s.tag.c_str(),s.marker,
                            s.size,aligned,unaligned);
            }
        }
        std::printf("SPR_ALIGN_SUMMARY resource=%s direct=%zu neighborPairs=%zu shiftedBestPairs=%zu needleAligned=%zu needleUnaligned=%zu\n",
                    name.c_str(),samples.size(),pairs,shiftWins,needleAligned,needleUnaligned);
    }
    std::puts("SPR_ALIGN_NOTE MSB-first exploratory comparisons only; a shifted match or 24-bit occurrence does not establish token boundaries, bit order, or codec semantics.");
    return failures?1:0;
}


// Milestone 232: read-only divergence localization between adjacent direct frames.
int stafuSprDivergenceSurvey(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-divergence: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-divergence: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-divergence: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Sample {
            size_t record,ordinal,offset,size;
            uint32_t a,b,marker;
            std::string tag;
        };
        std::vector<Sample> samples;
        for(size_t ri=0;ri<index.records().size();++ri) {
            size_t ordinal=0;
            for(const auto& e:index.records()[ri].events) {
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
                   !(v.marker==0x00030000&&v.size==4))
                    samples.push_back({ri,ordinal,v.offset,v.size,v.headerA,v.headerB,v.marker,e.tag});
                ++ordinal;
            }
        }
        size_t pairs=0,sharedSuffixPairs=0,localizedPairs=0,identicalPairs=0;
        size_t bitBoundaryCounts[8]={};
        for(size_t i=1;i<samples.size();++i) {
            const auto& a=samples[i-1];
            const auto& b=samples[i];
            if(a.record!=b.record||a.tag!=b.tag||a.marker!=b.marker||
               a.a!=b.a||a.b!=b.b)continue;
            ++pairs;
            const size_t common=std::min(a.size,b.size);
            size_t prefix=0;
            while(prefix<common&&bytes[a.offset+prefix]==bytes[b.offset+prefix])++prefix;
            size_t suffix=0;
            while(suffix<common-prefix&&
                  bytes[a.offset+a.size-1-suffix]==bytes[b.offset+b.size-1-suffix])++suffix;
            const size_t middleA=a.size-prefix-suffix;
            const size_t middleB=b.size-prefix-suffix;
            if(suffix)++sharedSuffixPairs;
            if(!middleA&&!middleB)++identicalPairs;
            // A localized difference means at least 75% of the longer payload
            // is explained by exact shared prefix and suffix, and both changed
            // middle regions are <= 64 bytes.
            const size_t longest=std::max(a.size,b.size);
            const bool localized=(middleA<=64&&middleB<=64&&
                (prefix+suffix)*4>=longest*3);
            if(localized)++localizedPairs;
            size_t bitInByte=0;
            if(prefix<common) {
                const uint8_t diff=bytes[a.offset+prefix]^bytes[b.offset+prefix];
                while(bitInByte<8&&!(diff&(0x80u>>bitInByte)))++bitInByte;
                if(bitInByte<8)++bitBoundaryCounts[bitInByte];
            }
            size_t sameTailWindow=0,comparedTailWindow=0;
            // Equal-position comparisons after the first differing byte,
            // independently of suffix alignment.
            const size_t end=std::min(common,prefix+1+256);
            for(size_t j=prefix+1;j<end;++j) {
                ++comparedTailWindow;
                if(bytes[a.offset+j]==bytes[b.offset+j])++sameTailWindow;
            }
            auto hexContext=[&](const Sample& s)->std::string {
                std::string out;
                const size_t start=prefix>4?prefix-4:0;
                const size_t end=std::min(s.size,prefix+9);
                char tmp[4];
                for(size_t j=start;j<end;++j) {
                    std::snprintf(tmp,sizeof(tmp),"%02x",bytes[s.offset+j]);
                    if(!out.empty())out+="_";
                    out+=tmp;
                }
                return out.empty()?"none":out;
            };
            std::printf("SPR_DIV_PAIR resource=%s record=%zu imageA=%zu imageB=%zu tag=%s marker=0x%08x dims=%u/%u sizeA=%zu sizeB=%zu prefixBytes=%zu firstDiffBit=%zu suffixBytes=%zu middleA=%zu middleB=%zu tailWindowSame=%zu tailWindowCompared=%zu localized=%u contextA=%s contextB=%s\n",
                        name.c_str(),a.record,a.ordinal,b.ordinal,a.tag.c_str(),a.marker,
                        a.a,a.b,a.size,b.size,prefix,
                        prefix<common?prefix*8+bitInByte:common*8,
                        suffix,middleA,middleB,sameTailWindow,comparedTailWindow,
                        localized?1u:0u,hexContext(a).c_str(),hexContext(b).c_str());
        }
        std::printf("SPR_DIV_SUMMARY resource=%s direct=%zu neighborPairs=%zu sharedSuffixPairs=%zu localizedPairs=%zu identicalPairs=%zu firstDifferenceBitWithinByte=",
                    name.c_str(),samples.size(),pairs,sharedSuffixPairs,localizedPairs,identicalPairs);
        for(size_t k=0;k<8;++k)std::printf("%s%zu:%zu",k?",":"",k,bitBoundaryCounts[k]);
        std::printf("\n");
    }
    std::puts("SPR_DIV_NOTE exact prefixes/suffixes and fixed-position windows are descriptive; byte-aligned suffixes are not evidence of token boundaries or a codec.");
    return failures?1:0;
}


// Milestone 233: read-only matching-block survey. Matches are byte-exact,
// greedily selected, non-overlapping within each payload, and >= 16 bytes.
int stafuSprCommonBlocks(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-common-blocks: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    constexpr size_t seedSize=16;
    constexpr size_t maxReported=12;
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-common-blocks: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-common-blocks: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Sample {
            size_t record,ordinal,offset,size;
            uint32_t a,b,marker;
            std::string tag;
        };
        std::vector<Sample> samples;
        for(size_t ri=0;ri<index.records().size();++ri) {
            size_t ordinal=0;
            for(const auto& e:index.records()[ri].events) {
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
                   !(v.marker==0x00030000&&v.size==4))
                    samples.push_back({ri,ordinal,v.offset,v.size,v.headerA,v.headerB,v.marker,e.tag});
                ++ordinal;
            }
        }
        size_t pairs=0,pairsWithBlocks=0,pairsWithShiftedBlocks=0;
        for(size_t i=1;i<samples.size();++i) {
            const auto& a=samples[i-1];
            const auto& b=samples[i];
            if(a.record!=b.record||a.tag!=b.tag||a.marker!=b.marker||
               a.a!=b.a||a.b!=b.b)continue;
            ++pairs;
            // FNV-1a on fixed 16-byte seeds. Verify every candidate by
            // comparing actual bytes; hash equality alone is never a match.
            auto hashSeed=[&](size_t offset)->uint64_t {
                uint64_t h=14695981039346656037ull;
                for(size_t k=0;k<seedSize;++k) {
                    h^=bytes[offset+k];
                    h*=1099511628211ull;
                }
                return h;
            };
            std::map<uint64_t,std::vector<size_t>> positions;
            if(a.size>=seedSize)
                for(size_t p=0;p+seedSize<=a.size;++p)
                    positions[hashSeed(a.offset+p)].push_back(p);
            struct Block {size_t posA,posB,length;};
            std::vector<Block> candidates;
            if(b.size>=seedSize)for(size_t q=0;q+seedSize<=b.size;++q) {
                const auto it=positions.find(hashSeed(b.offset+q));
                if(it==positions.end())continue;
                for(size_t p:it->second) {
                    if(!std::equal(bytes.begin()+a.offset+p,
                                   bytes.begin()+a.offset+p+seedSize,
                                   bytes.begin()+b.offset+q))continue;
                    // Keep only the leftmost seed of each maximal exact
                    // match; this avoids duplicate overlapping candidates.
                    if(p&&q&&bytes[a.offset+p-1]==bytes[b.offset+q-1])continue;
                    size_t len=seedSize;
                    while(p+len<a.size&&q+len<b.size&&
                          bytes[a.offset+p+len]==bytes[b.offset+q+len])++len;
                    candidates.push_back({p,q,len});
                }
            }
            std::sort(candidates.begin(),candidates.end(),
                      [](const Block& x,const Block& y) {
                          if(x.length!=y.length)return x.length>y.length;
                          if(x.posA!=y.posA)return x.posA<y.posA;
                          return x.posB<y.posB;
                      });
            std::vector<Block> chosen;
            size_t matchedBytes=0;
            for(const auto& candidate:candidates) {
                bool overlap=false;
                for(const auto& previous:chosen) {
                    if(candidate.posA<previous.posA+previous.length&&
                       previous.posA<candidate.posA+candidate.length)overlap=true;
                    if(candidate.posB<previous.posB+previous.length&&
                       previous.posB<candidate.posB+candidate.length)overlap=true;
                    if(overlap)break;
                }
                if(overlap)continue;
                chosen.push_back(candidate);
                matchedBytes+=candidate.length;
                if(chosen.size()==maxReported)break;
            }
            if(!chosen.empty())++pairsWithBlocks;
            bool shifted=false;
            for(size_t j=0;j<chosen.size();++j) {
                const auto& block=chosen[j];
                if(block.posA!=block.posB)shifted=true;
                const long long displacement=static_cast<long long>(block.posB)-
                                             static_cast<long long>(block.posA);
                std::printf("SPR_BLOCK_MATCH resource=%s record=%zu imageA=%zu imageB=%zu rank=%zu posA=%zu posB=%zu displacement=%lld length=%zu\n",
                            name.c_str(),a.record,a.ordinal,b.ordinal,j+1,
                            block.posA,block.posB,displacement,block.length);
            }
            if(shifted)++pairsWithShiftedBlocks;
            std::printf("SPR_BLOCK_PAIR resource=%s record=%zu imageA=%zu imageB=%zu tag=%s marker=0x%08x dims=%u/%u sizeA=%zu sizeB=%zu blocks=%zu matchedBytes=%zu shifted=%u\n",
                        name.c_str(),a.record,a.ordinal,b.ordinal,a.tag.c_str(),a.marker,
                        a.a,a.b,a.size,b.size,chosen.size(),matchedBytes,shifted?1u:0u);
        }
        std::printf("SPR_BLOCK_SUMMARY resource=%s direct=%zu neighborPairs=%zu pairsWithBlocks=%zu pairsWithShiftedBlocks=%zu minBlockBytes=%zu maxBlocksPerPair=%zu\n",
                    name.c_str(),samples.size(),pairs,pairsWithBlocks,
                    pairsWithShiftedBlocks,seedSize,maxReported);
    }
    std::puts("SPR_BLOCK_NOTE exact byte matches only; greedy longest-first selection is not an optimal alignment, and shared blocks do not establish codec semantics.");
    return failures?1:0;
}


// Milestone 234: read-only displacement coherence across ordered exact matches.
// Uses maximal 16-byte seed matches, caps candidate work, and groups matches
// into monotonic, non-overlapping runs at identical displacement.
int stafuSprDisplacementCoherence(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-displacement-coherence: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    constexpr size_t seed=16,maxCandidates=100000,maxRunLines=16;
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-displacement-coherence: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-displacement-coherence: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Sample {size_t record,ordinal,offset,size;uint32_t a,b,marker;std::string tag;};
        std::vector<Sample> samples;
        for(size_t ri=0;ri<index.records().size();++ri) {
            size_t ordinal=0;
            for(const auto& e:index.records()[ri].events) {
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
                   !(v.marker==0x00030000&&v.size==4))
                    samples.push_back({ri,ordinal,v.offset,v.size,v.headerA,v.headerB,v.marker,e.tag});
                ++ordinal;
            }
        }
        size_t pairs=0,pairsWithRuns=0,pairsWithMultiBlockRuns=0,truncatedPairs=0;
        for(size_t i=1;i<samples.size();++i) {
            const auto& a=samples[i-1];const auto& b=samples[i];
            if(a.record!=b.record||a.tag!=b.tag||a.marker!=b.marker||
               a.a!=b.a||a.b!=b.b)continue;
            ++pairs;
            auto hash=[&](size_t offset) {
                uint64_t h=14695981039346656037ull;
                for(size_t k=0;k<seed;++k){h^=bytes[offset+k];h*=1099511628211ull;}
                return h;
            };
            std::map<uint64_t,std::vector<size_t>> positions;
            if(a.size>=seed)for(size_t p=0;p+seed<=a.size;++p)
                positions[hash(a.offset+p)].push_back(p);
            struct Block {size_t x,y,n;};
            std::vector<Block> candidates;
            bool truncated=false;
            if(b.size>=seed)for(size_t q=0;q+seed<=b.size&&!truncated;++q) {
                const auto it=positions.find(hash(b.offset+q));
                if(it==positions.end())continue;
                for(size_t p:it->second) {
                    if(!std::equal(bytes.begin()+a.offset+p,
                                   bytes.begin()+a.offset+p+seed,
                                   bytes.begin()+b.offset+q))continue;
                    if(p&&q&&bytes[a.offset+p-1]==bytes[b.offset+q-1])continue;
                    size_t n=seed;
                    while(p+n<a.size&&q+n<b.size&&
                          bytes[a.offset+p+n]==bytes[b.offset+q+n])++n;
                    candidates.push_back({p,q,n});
                    if(candidates.size()>=maxCandidates){truncated=true;break;}
                }
            }
            if(truncated)++truncatedPairs;
            // Select an ordered, non-overlapping chain by weighted interval
            // dynamic programming over the candidate match endpoints.
            // O(N^2) is avoided: use a Fenwick prefix-max over B endpoints,
            // processing matches in increasing A endpoint batches.
            std::sort(candidates.begin(),candidates.end(),[](const Block& x,const Block& y){
                if(x.x+x.n!=y.x+y.n)return x.x+x.n<y.x+y.n;
                if(x.y+x.n!=y.y+y.n)return x.y+x.n<y.y+y.n;
                return x.n>y.n;
            });
            struct State {size_t score=0,count=0;int last=-1;};
            std::vector<State> tree(b.size+2),dp(candidates.size());
            auto better=[](const State& x,const State& y) {
                return x.score>y.score||(x.score==y.score&&x.count<y.count);
            };
            auto query=[&](size_t pos) {
                State result;
                for(size_t j=pos+1;j;j-=j&-j)
                    if(better(tree[j],result))result=tree[j];
                return result;
            };
            auto update=[&](size_t pos,const State& value) {
                for(size_t j=pos+1;j<tree.size();j+=j&-j)
                    if(better(value,tree[j]))tree[j]=value;
            };
            std::vector<int> predecessor(candidates.size(),-1);
            size_t released=0;
            State best;
            for(size_t j=0;j<candidates.size();++j) {
                const auto& m=candidates[j];
                while(released<j&&candidates[released].x+candidates[released].n<=m.x) {
                    update(candidates[released].y+candidates[released].n,dp[released]);
                    ++released;
                }
                const State prev=query(m.y);
                predecessor[j]=prev.last;
                dp[j]={prev.score+m.n,prev.count+1,static_cast<int>(j)};
                if(better(dp[j],best))best=dp[j];
            }
            std::vector<Block> chain;
            for(int j=best.last;j>=0;j=predecessor[static_cast<size_t>(j)])
                chain.push_back(candidates[static_cast<size_t>(j)]);
            std::reverse(chain.begin(),chain.end());
            struct Run {size_t firstA,firstB,endA,endB,bytes,blocks;long long delta;};
            std::vector<Run> runs;
            for(const auto& m:chain) {
                const long long d=static_cast<long long>(m.y)-static_cast<long long>(m.x);
                if(!runs.empty()&&runs.back().delta==d) {
                    auto& run=runs.back();run.endA=m.x+m.n;run.endB=m.y+m.n;
                    run.bytes+=m.n;++run.blocks;
                } else runs.push_back({m.x,m.y,m.x+m.n,m.y+m.n,m.n,1,d});
            }
            if(!runs.empty())++pairsWithRuns;
            bool multi=false;for(const auto& run:runs)if(run.blocks>1)multi=true;
            if(multi)++pairsWithMultiBlockRuns;
            size_t matched=0;for(const auto& m:chain)matched+=m.n;
            std::printf("SPR_COHERENCE_PAIR resource=%s record=%zu imageA=%zu imageB=%zu tag=%s marker=0x%08x dims=%u/%u sizeA=%zu sizeB=%zu candidates=%zu truncated=%u chainBlocks=%zu chainBytes=%zu runs=%zu multiBlock=%u\n",
                        name.c_str(),a.record,a.ordinal,b.ordinal,a.tag.c_str(),a.marker,
                        a.a,a.b,a.size,b.size,candidates.size(),truncated?1u:0u,
                        chain.size(),matched,runs.size(),multi?1u:0u);
            for(size_t k=0;k<std::min(runs.size(),maxRunLines);++k) {
                const auto& run=runs[k];
                const long long change=k?run.delta-runs[k-1].delta:0;
                std::printf("SPR_COHERENCE_RUN resource=%s record=%zu imageA=%zu imageB=%zu run=%zu blocks=%zu matchedBytes=%zu startA=%zu endA=%zu startB=%zu endB=%zu displacement=%lld deltaFromPrevious=%lld\n",
                            name.c_str(),a.record,a.ordinal,b.ordinal,k+1,run.blocks,
                            run.bytes,run.firstA,run.endA,run.firstB,run.endB,run.delta,change);
            }
        }
        std::printf("SPR_COHERENCE_SUMMARY resource=%s direct=%zu neighborPairs=%zu pairsWithRuns=%zu pairsWithMultiBlockRuns=%zu truncatedPairs=%zu minMatchBytes=%zu maxReportedRuns=%zu\n",
                    name.c_str(),samples.size(),pairs,pairsWithRuns,pairsWithMultiBlockRuns,
                    truncatedPairs,seed,maxRunLines);
    }
    std::puts("SPR_COHERENCE_NOTE byte-exact ordered matches; greedy/weighted alignment is descriptive, and displacement transitions are not proven codec boundaries.");
    return failures?1:0;
}


// Milestone 235: read-only alignment coverage and candidate ambiguity.
// Uses maximal 16-byte seed matches, caps candidate work, and groups matches
// into monotonic, non-overlapping runs at identical displacement.
int stafuSprAlignmentCoverage(const char* installed,const char* cd,const char* optionalName) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if(!manager.open(installed,cd,error)) {
        std::fprintf(stderr,"spr-alignment-coverage: %s\n",error.c_str());return 1;
    }
    const std::vector<std::string> names=optionalName
        ?std::vector<std::string>{optionalName}
        :std::vector<std::string>{"BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR",
                                  "BRDGTROI.SPR","BRDGWORF.SPR"};
    constexpr size_t seed=16,maxCandidates=100000,maxRunLines=16;
    int failures=0;
    for(const auto& name:names) {
        std::vector<uint8_t> bytes;
        if(!manager.read(name,bytes,error)) {
            std::fprintf(stderr,"spr-alignment-coverage: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        reluva::stafu::SprSequenceIndex index;
        if(!index.build(bytes,error)) {
            std::fprintf(stderr,"spr-alignment-coverage: %s: %s\n",name.c_str(),error.c_str());
            ++failures;continue;
        }
        struct Sample {size_t record,ordinal,offset,size;uint32_t a,b,marker;std::string tag;};
        std::vector<Sample> samples;
        for(size_t ri=0;ri<index.records().size();++ri) {
            size_t ordinal=0;
            for(const auto& e:index.records()[ri].events) {
                if(e.tag!="PMOC"&&e.tag!="MOCS")continue;
                reluva::stafu::SprImagePayloadAccessor::View v;
                if(reluva::stafu::SprImagePayloadAccessor::get(bytes,e,v)&&
                   !(v.marker==0x00030000&&v.size==4))
                    samples.push_back({ri,ordinal,v.offset,v.size,v.headerA,v.headerB,v.marker,e.tag});
                ++ordinal;
            }
        }
        size_t pairs=0,pairsWithRuns=0,pairsWithMultiBlockRuns=0,truncatedPairs=0,ambiguousPairs=0;
        size_t totalMatched=0,totalA=0,totalB=0;
        for(size_t i=1;i<samples.size();++i) {
            const auto& a=samples[i-1];const auto& b=samples[i];
            if(a.record!=b.record||a.tag!=b.tag||a.marker!=b.marker||
               a.a!=b.a||a.b!=b.b)continue;
            ++pairs;
            auto hash=[&](size_t offset) {
                uint64_t h=14695981039346656037ull;
                for(size_t k=0;k<seed;++k){h^=bytes[offset+k];h*=1099511628211ull;}
                return h;
            };
            std::map<uint64_t,std::vector<size_t>> positions;
            if(a.size>=seed)for(size_t p=0;p+seed<=a.size;++p)
                positions[hash(a.offset+p)].push_back(p);
            struct Block {size_t x,y,n;};
            std::vector<Block> candidates;
            bool truncated=false;
            if(b.size>=seed)for(size_t q=0;q+seed<=b.size&&!truncated;++q) {
                const auto it=positions.find(hash(b.offset+q));
                if(it==positions.end())continue;
                for(size_t p:it->second) {
                    if(!std::equal(bytes.begin()+a.offset+p,
                                   bytes.begin()+a.offset+p+seed,
                                   bytes.begin()+b.offset+q))continue;
                    if(p&&q&&bytes[a.offset+p-1]==bytes[b.offset+q-1])continue;
                    size_t n=seed;
                    while(p+n<a.size&&q+n<b.size&&
                          bytes[a.offset+p+n]==bytes[b.offset+q+n])++n;
                    candidates.push_back({p,q,n});
                    if(candidates.size()>=maxCandidates){truncated=true;break;}
                }
            }
            if(truncated)++truncatedPairs;
            // Select an ordered, non-overlapping chain by weighted interval
            // dynamic programming over the candidate match endpoints.
            // O(N^2) is avoided: use a Fenwick prefix-max over B endpoints,
            // processing matches in increasing A endpoint batches.
            std::sort(candidates.begin(),candidates.end(),[](const Block& x,const Block& y){
                if(x.x+x.n!=y.x+y.n)return x.x+x.n<y.x+y.n;
                if(x.y+x.n!=y.y+y.n)return x.y+x.n<y.y+y.n;
                return x.n>y.n;
            });
            struct State {size_t score=0,count=0;int last=-1;};
            std::vector<State> tree(b.size+2),dp(candidates.size());
            auto better=[](const State& x,const State& y) {
                return x.score>y.score||(x.score==y.score&&x.count<y.count);
            };
            auto query=[&](size_t pos) {
                State result;
                for(size_t j=pos+1;j;j-=j&-j)
                    if(better(tree[j],result))result=tree[j];
                return result;
            };
            auto update=[&](size_t pos,const State& value) {
                for(size_t j=pos+1;j<tree.size();j+=j&-j)
                    if(better(value,tree[j]))tree[j]=value;
            };
            std::vector<int> predecessor(candidates.size(),-1);
            size_t released=0;
            State best;
            for(size_t j=0;j<candidates.size();++j) {
                const auto& m=candidates[j];
                while(released<j&&candidates[released].x+candidates[released].n<=m.x) {
                    update(candidates[released].y+candidates[released].n,dp[released]);
                    ++released;
                }
                const State prev=query(m.y);
                predecessor[j]=prev.last;
                dp[j]={prev.score+m.n,prev.count+1,static_cast<int>(j)};
                if(better(dp[j],best))best=dp[j];
            }
            std::vector<Block> chain;
            for(int j=best.last;j>=0;j=predecessor[static_cast<size_t>(j)])
                chain.push_back(candidates[static_cast<size_t>(j)]);
            std::reverse(chain.begin(),chain.end());
            struct Run {size_t firstA,firstB,endA,endB,bytes,blocks;long long delta;};
            std::vector<Run> runs;
            for(const auto& m:chain) {
                const long long d=static_cast<long long>(m.y)-static_cast<long long>(m.x);
                if(!runs.empty()&&runs.back().delta==d) {
                    auto& run=runs.back();run.endA=m.x+m.n;run.endB=m.y+m.n;
                    run.bytes+=m.n;++run.blocks;
                } else runs.push_back({m.x,m.y,m.x+m.n,m.y+m.n,m.n,1,d});
            }
            if(!runs.empty())++pairsWithRuns;
            bool multi=false;for(const auto& run:runs)if(run.blocks>1)multi=true;
            if(multi)++pairsWithMultiBlockRuns;
            size_t matched=0;for(const auto& m:chain)matched+=m.n;
            // Gap sizes are measured between selected ordered blocks, with
            // leading and trailing gaps included in total unmatched bytes.
            size_t maxGapA=0,maxGapB=0,internalGapA=0,internalGapB=0;
            size_t prevA=0,prevB=0;
            for(size_t k=0;k<chain.size();++k) {
                const auto& m=chain[k];
                const size_t gapA=m.x-prevA,gapB=m.y-prevB;
                maxGapA=std::max(maxGapA,gapA);maxGapB=std::max(maxGapB,gapB);
                if(k){internalGapA=std::max(internalGapA,gapA);
                      internalGapB=std::max(internalGapB,gapB);}
                prevA=m.x+m.n;prevB=m.y+m.n;
            }
            maxGapA=std::max(maxGapA,a.size-prevA);
            maxGapB=std::max(maxGapB,b.size-prevB);
            // An alternative candidate is ambiguous only when it has the
            // same A start and exact length as a selected block, but a
            // different B start. This is a conservative local ambiguity
            // measure, not a count of globally optimal alternative chains.
            size_t ambiguousBlocks=0,alternativePositions=0;
            for(const auto& m:chain) {
                size_t alternatives=0;
                for(const auto& c:candidates)
                    if(c.x==m.x&&c.n==m.n&&c.y!=m.y)++alternatives;
                if(alternatives){++ambiguousBlocks;alternativePositions+=alternatives;}
            }
            if(ambiguousBlocks)++ambiguousPairs;
            totalMatched+=matched;totalA+=a.size;totalB+=b.size;
            const size_t unmatchedA=a.size-matched,unmatchedB=b.size-matched;
            const double coverageA=a.size?100.0*double(matched)/double(a.size):100.0;
            const double coverageB=b.size?100.0*double(matched)/double(b.size):100.0;
            std::printf("SPR_COVERAGE_METRICS resource=%s record=%zu imageA=%zu imageB=%zu matchedBytes=%zu coverageA=%.2f coverageB=%.2f unmatchedA=%zu unmatchedB=%zu maxGapA=%zu maxGapB=%zu maxInternalGapA=%zu maxInternalGapB=%zu ambiguousBlocks=%zu alternativePositions=%zu\\n",
                        name.c_str(),a.record,a.ordinal,b.ordinal,matched,coverageA,coverageB,
                        unmatchedA,unmatchedB,maxGapA,maxGapB,internalGapA,internalGapB,
                        ambiguousBlocks,alternativePositions);
            std::printf("SPR_COVERAGE_PAIR resource=%s record=%zu imageA=%zu imageB=%zu tag=%s marker=0x%08x dims=%u/%u sizeA=%zu sizeB=%zu candidates=%zu truncated=%u chainBlocks=%zu chainBytes=%zu runs=%zu multiBlock=%u\n",
                        name.c_str(),a.record,a.ordinal,b.ordinal,a.tag.c_str(),a.marker,
                        a.a,a.b,a.size,b.size,candidates.size(),truncated?1u:0u,
                        chain.size(),matched,runs.size(),multi?1u:0u);
            for(size_t k=0;k<std::min(runs.size(),maxRunLines);++k) {
                const auto& run=runs[k];
                const long long change=k?run.delta-runs[k-1].delta:0;
                std::printf("SPR_COVERAGE_RUN resource=%s record=%zu imageA=%zu imageB=%zu run=%zu blocks=%zu matchedBytes=%zu startA=%zu endA=%zu startB=%zu endB=%zu displacement=%lld deltaFromPrevious=%lld\n",
                            name.c_str(),a.record,a.ordinal,b.ordinal,k+1,run.blocks,
                            run.bytes,run.firstA,run.endA,run.firstB,run.endB,run.delta,change);
            }
        }
        std::printf("SPR_COVERAGE_SUMMARY resource=%s direct=%zu neighborPairs=%zu pairsWithRuns=%zu pairsWithMultiBlockRuns=%zu truncatedPairs=%zu minMatchBytes=%zu maxReportedRuns=%zu\n",
                    name.c_str(),samples.size(),pairs,pairsWithRuns,pairsWithMultiBlockRuns,
                    truncatedPairs,ambiguousPairs,totalMatched,totalA,totalB,seed,maxRunLines);
    }
    std::puts("SPR_COVERAGE_NOTE coverage uses one maximum-weight ordered chain of byte-exact matches >=16 bytes; ambiguity counts same-A-start same-length alternatives only; candidate cap and tie-breaking can affect results; no codec boundaries inferred.");
    return failures?1:0;
}
