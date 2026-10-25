// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only discovery of candidate palette resources; names alone do not prove usage.
#include "engines/stafu/zip_archive.h"
#include "engines/stafu/rm_palette_discovery.h"
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>
namespace fs=std::filesystem;
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-palette-discovery <installed-folder> <cd-folder>\n";return 2;}
 std::set<std::string> names;
 size_t loose=0,archive=0;
 std::error_code ec;
 const fs::path root(argv[1]);
 if(!fs::is_directory(root,ec)){std::cerr<<"Installed folder unavailable\n";return 1;}
 for(fs::recursive_directory_iterator it(root,fs::directory_options::skip_permission_denied,ec),end;it!=end;it.increment(ec)){
  if(ec){std::cerr<<"Directory traversal error: "<<ec.message()<<"\n";return 1;}
  if(!it->is_regular_file(ec))continue;
  const auto name=it->path().filename().string();
  if(reluva::stafu::rmPaletteCandidate(name)){
   std::cout<<"LOOSE "<<it->path().lexically_relative(root).string()
            <<" bytes="<<it->file_size(ec)<<"\n";
   names.insert(name);++loose;
  }
 }
 for(const char* archiveName:{"STTNG.ZIP","STTNGINS.ZIP"}){
  reluva::stafu::ZipArchive zip;std::string error;
  if(!zip.open(fs::path(argv[2])/archiveName,error)){
   std::cout<<"ARCHIVE_UNAVAILABLE "<<archiveName<<" reason="<<error<<"\n";continue;
  }
  for(const auto& e:zip.entries()){
   if(!reluva::stafu::rmPaletteCandidate(e.name))continue;
   std::cout<<"ARCHIVE "<<archiveName<<" "<<e.name<<" bytes="<<e.uncompressedSize<<"\n";
   names.insert(e.name);++archive;
  }
 }
 std::cout<<"SUMMARY looseCandidates="<<loose<<" archiveCandidates="<<archive
          <<" uniqueNames="<<names.size()<<"\n";
 std::cout<<"NOTE: Candidate names and sizes do not establish palette formats or RM associations.\n";
 return 0;
}
