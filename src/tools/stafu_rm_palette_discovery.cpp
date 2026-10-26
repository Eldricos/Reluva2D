// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only discovery of candidate palette resources; names alone do not prove usage.
#include "engines/stafu/zip_archive.h"
#include "engines/stafu/rm_palette_discovery.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>
namespace fs=std::filesystem;

namespace {
void report(const std::string& source,const std::string& name,const std::vector<uint8_t>& data){
 const auto s=reluva::stafu::rmPaletteSignature(data.data(),data.size());
 std::cout<<"PROVENANCE source="<<source<<" name="<<name<<" bytes="<<data.size()
          <<" rawRgb256="<<s.rawRgb256<<" vga6_256="<<s.vga6_256
          <<" vga6_128="<<s.vga6_128<<" gifGlobal256="<<s.gifGlobal256<<"\n";
}
bool inspectLoose(const fs::path& path,std::vector<uint8_t>& data){
 std::error_code ec;
 const auto size=fs::file_size(path,ec);
 if(ec||size>1024*1024)return false;
 std::ifstream in(path,std::ios::binary);
 if(!in)return false;
 data.resize(size_t(size));
 if(size&&!in.read(reinterpret_cast<char*>(data.data()),std::streamsize(size)))return false;
 return true;
}
}

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
   std::vector<uint8_t> data;
   if(inspectLoose(it->path(),data))
    report("loose",it->path().lexically_relative(root).generic_string(),data);
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
   if(e.uncompressedSize<=1024*1024){
    std::vector<uint8_t> data;std::string readError;
    if(zip.read(e.name,data,readError,1024*1024))
     report(archiveName,e.name,data);
    else std::cout<<"PROVENANCE_READ_ERROR "<<archiveName<<" "<<e.name<<" "<<readError<<"\n";
   }
  }
 }
 for(const char* img:{"BRIDGE.IMG","DBRIDGE.IMG","color.img"}){
  for(const char* archiveName:{"STTNG.ZIP","STTNGINS.ZIP"}){
   reluva::stafu::ZipArchive zip;std::string error;
   if(!zip.open(fs::path(argv[2])/archiveName,error))continue;
   const auto* entry=zip.find(img);
   if(!entry||entry->uncompressedSize>1024*1024)continue;
   std::vector<uint8_t> data;
   if(zip.read(img,data,error,1024*1024))report(archiveName,img,data);
  }
 }
 std::cout<<"SUMMARY looseCandidates="<<loose<<" archiveCandidates="<<archive
          <<" uniqueNames="<<names.size()<<"\n";
 std::cout<<"NOTE: Candidate names and sizes do not establish palette formats or RM associations.\n";
 return 0;
}
