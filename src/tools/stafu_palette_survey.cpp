// SPDX-License-Identifier: GPL-3.0-or-later
// Milestone 65: read-only palette resource survey; prints candidate metadata, not game assets.
#include "engines/stafu/zip_archive.h"
#include "engines/stafu/rm_palette_inventory.h"
#include "engines/stafu/rm_palette_scan.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
namespace fs=std::filesystem;
struct Candidate {std::string origin,name;uint64_t size=0;unsigned score=0;};
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-palette-survey <installed-folder> <cd-folder>\n";return 2;}
 std::vector<Candidate> candidates;
 const fs::path installed=argv[1],cd=argv[2];
 std::error_code ec;
 if(!fs::is_directory(installed,ec)){std::cerr<<"Installed folder unavailable\n";return 1;}
 size_t loose=0,archived=0;
 for(fs::recursive_directory_iterator it(installed,fs::directory_options::skip_permission_denied,ec),end;
     it!=end&&!ec;it.increment(ec)){
  if(!it->is_regular_file(ec))continue;
  ++loose;
  const auto name=it->path().filename().string();
  const auto size=it->file_size(ec);
  if(ec){ec.clear();continue;}
  const auto score=reluva::stafu::paletteResourceScore(name,size);
  if(score)candidates.push_back({"installed",name,size,score});
 }
 if(ec)std::cout<<"WARNING installed inventory incomplete: "<<ec.message()<<"\n";
 const fs::path archive=cd/"STTNG.ZIP";
 reluva::stafu::ZipArchive zip;
 std::string error;
 if(!zip.open(archive,error)){std::cerr<<"Archive unavailable: "<<error<<"\n";return 1;}
 for(const auto& entry:zip.entries()){
  ++archived;
  const auto score=reluva::stafu::paletteResourceScore(entry.name,entry.uncompressedSize);
  if(score)candidates.push_back({"archive",entry.name,entry.uncompressedSize,score});
 }
 std::sort(candidates.begin(),candidates.end(),[](const auto& a,const auto& b){
  if(a.score!=b.score)return a.score>b.score;
  if(a.name!=b.name)return a.name<b.name;
  return a.origin<b.origin;
 });
 std::cout<<"INVENTORY loose="<<loose<<" archive="<<archived
          <<" rankedCandidates="<<candidates.size()<<"\n";
 const size_t maxReport=80;
 for(size_t i=0;i<std::min(maxReport,candidates.size());++i){
  const auto& c=candidates[i];
  std::cout<<"CANDIDATE origin="<<c.origin<<" name="<<c.name
           <<" bytes="<<c.size<<" score="<<c.score;
  // Only scan small high-priority archive entries, bounded to 2 MiB.
  if(c.origin=="archive"&&c.score>=8&&c.size>=768&&c.size<=2*1024*1024){
   std::vector<uint8_t> bytes;std::string readError;
   if(zip.read(c.name,bytes,readError,2*1024*1024)){
    const auto hits=reluva::stafu::scanVgaPaletteWindows(bytes.data(),bytes.size(),3);
    std::cout<<" vga6Hits="<<hits.size();
    for(const auto& hit:hits)std::cout<<" offset="<<hit.offset;
   }else std::cout<<" scanError="<<readError;
  }
  std::cout<<"\n";
 }
 if(candidates.size()>maxReport)std::cout<<"TRUNCATED "<<(candidates.size()-maxReport)<<" candidates not displayed\n";
 std::cout<<"NOTE: Scores, sizes and VGA6 patterns are leads, not proof of palette ownership.\n";
 return 0;
}
