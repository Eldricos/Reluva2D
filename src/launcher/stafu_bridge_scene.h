// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Milestone 56: SDL3 bridge presentation adapter. No gameplay or asset mutations.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_gif_rgba.h"
#include "engines/stafu/rm_grayscale.h"
#include "engines/stafu/rm_section_align.h"
#include "engines/stafu/rm_section_assembly.h"
#include "engines/stafu/rm_working_geometry.h"
#include "engines/stafu/rm_candidate_palette.h"
#include "engines/stafu/rm_standard_palette.h"
#include <SDL3/SDL.h>
#include <imgui.h>
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace reluva {
class StafuBridgeScene {
public:
    StafuBridgeScene(SDL_Renderer* renderer,const stafu::ResourceManager& resources)
        : renderer_(renderer) {
        load(0,"BRIDGE.IMG",resources);
        load(1,"DBRIDGE.IMG",resources);
        loadPalette(0,"BRIDGE.IMG",resources);
        loadPalette(1,"DBRIDGE.IMG",resources);
        loadPalette(2,"color.img",resources);
        loadStandardPalette(resources);
        loadRm(0,"BRIDGE.RM",resources);
        loadRm(1,"VIEWSCR.RM",resources);
    }
    ~StafuBridgeScene() {
        for(auto& item:images_)if(item.texture)SDL_DestroyTexture(item.texture);
        for(auto& item:rmImages_)if(item.texture)SDL_DestroyTexture(item.texture);
    }
    StafuBridgeScene(const StafuBridgeScene&)=delete;
    StafuBridgeScene& operator=(const StafuBridgeScene&)=delete;

    void draw() {
        ImGui::TextUnformatted("Bridge scene - resource presentation foundation");
        ImGui::Checkbox("Experimental RM grayscale viewer",&showRm_);
        if(showRm_) { drawRm();return; }
        ImGui::TextWrapped("The two verified files appear to depict normal and damaged states of the same panel. "
                           "This identification is provisional; their in-game placement is not established.");
        if(ImGui::RadioButton("Normal panel (BRIDGE.IMG)",selected_==0))selected_=0;
        if(ImGui::RadioButton("Damaged panel (DBRIDGE.IMG)",selected_==1))selected_=1;
        ImGui::Separator();
        const auto& item=images_[selected_];
        if(!item.texture) {
            ImGui::TextDisabled("Resource unavailable: %s",item.error.c_str());
        } else {
            float w=0.0f,h=0.0f;
            if(SDL_GetTextureSize(item.texture,&w,&h)) {
                const ImVec2 available=ImGui::GetContentRegionAvail();
                // Integer enlargement only, with nearest-neighbor sampling.
                const int sx=static_cast<int>(available.x/w);
                const int sy=static_cast<int>(available.y/h);
                const int scale=std::max(1,std::min(8,std::min(sx,sy)));
                ImGui::Image(reinterpret_cast<ImTextureID>(item.texture),
                             ImVec2(w*scale,h*scale));
            }
        }
        ImGui::TextDisabled("Original game resource preview; scene geometry and interactions are not yet decoded.");
    }

private:
    struct Image {
        SDL_Texture* texture=nullptr;
        std::string error;
    };
    void load(size_t index,const char* name,const stafu::ResourceManager& resources) {
        auto& item=images_[index];
        std::vector<uint8_t> bytes;
        if(!resources.read(name,bytes,item.error))return;
        const auto decoded=stafu::decodeSprGifRgba(bytes.data(),bytes.size());
        if(!decoded.valid){item.error=decoded.error;return;}
        item.texture=SDL_CreateTexture(renderer_,SDL_PIXELFORMAT_RGBA32,
                                       SDL_TEXTUREACCESS_STATIC,decoded.width,decoded.height);
        if(!item.texture){item.error=SDL_GetError();return;}
        if(!SDL_UpdateTexture(item.texture,nullptr,decoded.pixels.data(),int(decoded.width)*4)){
            item.error=SDL_GetError();
            SDL_DestroyTexture(item.texture);
            item.texture=nullptr;
            return;
        }
        SDL_SetTextureScaleMode(item.texture,SDL_SCALEMODE_NEAREST);
    }
    void loadPalette(size_t index,const char* name,const stafu::ResourceManager& resources) {
        std::vector<uint8_t> bytes;
        std::string error;
        if(!resources.read(name,bytes,error)){paletteErrors_[index]=error;return;}
        palettes_[index]=stafu::extractRmGifPalette(bytes.data(),bytes.size());
        if(!palettes_[index].valid)paletteErrors_[index]=palettes_[index].error;
    }
    void loadStandardPalette(const stafu::ResourceManager& resources) {
        if(!resources.read("standard.pal",standardPalette_,standardPaletteError_,4096))return;
        if(standardPalette_.size()!=384){standardPaletteError_="Expected 384 bytes";standardPalette_.clear();return;}
        for(const auto value:standardPalette_)if(value>63){standardPaletteError_="Not a VGA6 table";standardPalette_.clear();return;}
    }
    void loadRm(size_t index,const char* name,const stafu::ResourceManager& resources) {
        auto& item=rmImages_[index];
        std::vector<uint8_t> bytes;
        if(!resources.read(name,bytes,item.error))return;
        const auto decoded=stafu::decodeRmGrayscale(bytes.data(),bytes.size());
        if(!decoded.valid){item.error=decoded.error;return;}
        rmOriginal_[index]=decoded.rgba;
        item.texture=SDL_CreateTexture(renderer_,SDL_PIXELFORMAT_RGBA32,
                                       SDL_TEXTUREACCESS_STATIC,int(decoded.width),int(decoded.height));
        if(!item.texture){item.error=SDL_GetError();return;}
        if(!SDL_UpdateTexture(item.texture,nullptr,decoded.rgba.data(),int(decoded.width)*4)){
            item.error=SDL_GetError();
            SDL_DestroyTexture(item.texture);
            item.texture=nullptr;return;
        }
        SDL_SetTextureScaleMode(item.texture,SDL_SCALEMODE_NEAREST);
        updateRmShift(index);
    }
    void updateRmShift(size_t index) {
        auto& item=rmImages_[index];
        if(!item.texture||rmOriginal_[index].empty())return;
        std::vector<uint8_t> source=rmOriginal_[index];
        if(rmPaletteMode_==3&&!standardPalette_.empty()) {
            std::vector<uint8_t> indices(source.size()/4);
            for(size_t i=0;i<indices.size();++i)indices[i]=source[i*4];
            std::vector<uint8_t> mapped;
            if(stafu::mapStandardPal(indices.data(),indices.size(),standardPalette_.data(),standardPalette_.size(),mapped))source=std::move(mapped);
        }else if(rmPaletteMode_==1||rmPaletteMode_==2||rmPaletteMode_==4) {
            const auto& palette=palettes_[rmPaletteMode_==4?2:rmPaletteMode_-1];
            if(palette.valid) {
                for(size_t p=0;p<source.size();p+=4){
                    const size_t color=static_cast<size_t>(source[p])*3;
                    if(color/3<palette.entries){
                        source[p]=palette.rgb[color];
                        source[p+1]=palette.rgb[color+1];
                        source[p+2]=palette.rgb[color+2];
                    }
                }
            }
        }
        std::vector<uint8_t> shifted;
        if(!rmGeometryDebug_) {
            if(!stafu::decodeRmWorkingGeometryRgba(source.data(),source.size(),shifted))return;
        } else if(rmAssemblyMode_ && rmShift_[index]==256) {
            if(!stafu::assembleRmSectionsRgba(source.data(),source.size(),
                                              640,480,384,rmVertical_[index],rmRightVertical_[index],shifted))return;
        } else {
            if(!stafu::alignRmRgba(source.data(),source.size(),
                                    640,480,rmShift_[index],rmVertical_[index],shifted))return;
        }
        if(!SDL_UpdateTexture(item.texture,nullptr,shifted.data(),640*4))
            item.error=SDL_GetError();
    }
    void drawRm() {
        ImGui::TextWrapped("RM working geometry: 640 x 480, rotate right 256, left section down 1. "
                           "Alignment visually verified; original color palette remains unknown.");
        if(ImGui::RadioButton("BRIDGE.RM",rmSelected_==0))rmSelected_=0;
        ImGui::SameLine();
        if(ImGui::RadioButton("VIEWSCR.RM",rmSelected_==1))rmSelected_=1;
        ImGui::TextDisabled("Palette preview is experimental; IMG palettes are not verified as RM palettes.");
        bool paletteChanged=false;
        if(ImGui::RadioButton("Grayscale (verified baseline)",rmPaletteMode_==0)){
            rmPaletteMode_=0;paletteChanged=true;
        }
        if(ImGui::RadioButton("BRIDGE.IMG palette",rmPaletteMode_==1)){
            rmPaletteMode_=1;paletteChanged=true;
        }
        ImGui::SameLine();
        if(ImGui::RadioButton("DBRIDGE.IMG palette",rmPaletteMode_==2)){
            rmPaletteMode_=2;paletteChanged=true;
        }
        if(ImGui::RadioButton("standard.pal (128 colors; unsupported indices magenta)",rmPaletteMode_==3)){
            rmPaletteMode_=3;paletteChanged=true;
        }
        if(ImGui::RadioButton("color.img GIF palette (experimental)",rmPaletteMode_==4)){
            rmPaletteMode_=4;paletteChanged=true;
        }
        if(rmPaletteMode_==3&&standardPalette_.empty())
            ImGui::TextDisabled("standard.pal unavailable: %s",standardPaletteError_.c_str());
        if((rmPaletteMode_==1||rmPaletteMode_==2||rmPaletteMode_==4)&&!palettes_[rmPaletteMode_==4?2:rmPaletteMode_-1].valid)
            ImGui::TextDisabled("Palette unavailable: %s",paletteErrors_[rmPaletteMode_==4?2:rmPaletteMode_-1].c_str());
        if(paletteChanged){
            updateRmShift(0);
            updateRmShift(1);
        }
        if(ImGui::Checkbox("Enable geometry debugging controls",&rmGeometryDebug_)) {
            updateRmShift(0);updateRmShift(1);
        }
        if(rmGeometryDebug_) {
        ImGui::TextDisabled("Shift direction: positive moves pixels right; negative moves pixels left.");
        bool changed=ImGui::SliderInt("Horizontal shift (pixels)",&rmShift_[rmSelected_],-639,639);
        if(ImGui::Button("Left 1")){--rmShift_[rmSelected_];changed=true;}
        ImGui::SameLine();
        if(ImGui::Button("Right 1")){++rmShift_[rmSelected_];changed=true;}
        ImGui::SameLine();
        if(ImGui::Button("Reset shift")){rmShift_[rmSelected_]=0;changed=true;}
        if(changed) {
            // Preserve a bounded display range, including button presses at slider limits.
            rmShift_[rmSelected_]=std::clamp(rmShift_[rmSelected_],-639,639);
            updateRmShift(static_cast<size_t>(rmSelected_));
        }
        ImGui::TextDisabled("Vertical shift applies only to the wrapped LEFT section; positive moves it down.");
        bool verticalChanged=ImGui::SliderInt("Left section vertical shift",&rmVertical_[rmSelected_],-16,16);
        if(ImGui::Button("Left section Up 1")){--rmVertical_[rmSelected_];verticalChanged=true;}
        ImGui::SameLine();
        if(ImGui::Button("Left section Down 1")){++rmVertical_[rmSelected_];verticalChanged=true;}
        ImGui::SameLine();
        if(ImGui::Button("Reset vertical")){rmVertical_[rmSelected_]=0;verticalChanged=true;}
        if(verticalChanged){
            rmVertical_[rmSelected_]=std::clamp(rmVertical_[rmSelected_],-16,16);
            updateRmShift(static_cast<size_t>(rmSelected_));
        }
        ImGui::Separator();
        if(ImGui::Checkbox("Experimental split-section assembly",&rmAssemblyMode_)) {
            updateRmShift(0);updateRmShift(1);
        }
        if(rmAssemblyMode_) {
            ImGui::TextWrapped("Split model: raw columns 384-639 become display left; "
                               "raw columns 0-383 become display right. Independent vertical offsets; "
                               "horizontal shift must be +256.");
            if(rmShift_[rmSelected_]!=256) {
                ImGui::TextDisabled("Horizontal shift is not +256: displaying baseline rotation instead.");
            } else if(ImGui::SliderInt("Right section vertical shift",&rmRightVertical_[rmSelected_],-16,16)) {
                updateRmShift(static_cast<size_t>(rmSelected_));
            }
            if(ImGui::Button("Reset right section vertical")) {
                rmRightVertical_[rmSelected_]=0;
                updateRmShift(static_cast<size_t>(rmSelected_));
            }
            ImGui::TextDisabled("At right shift 0, split assembly matches the existing +256 rotation with left vertical shift.");
        }
        } else {
            ImGui::TextDisabled("Using verified geometry (+256 horizontal, +1 left vertical, 0 right vertical).");
        }
        ImGui::Separator();
        const auto& item=rmImages_[rmSelected_];
        if(!item.texture){ImGui::TextDisabled("Resource unavailable: %s",item.error.c_str());return;}
        // Scrollable viewport preserves the full 640x480 image at native pixel size.
        ImGui::BeginChild("RM raster viewport",ImVec2(0,0),ImGuiChildFlags_Borders);
        ImGui::Image(reinterpret_cast<ImTextureID>(item.texture),ImVec2(640,480));
        ImGui::EndChild();
    }
    SDL_Renderer* renderer_;
    Image images_[2];
    Image rmImages_[2];
    std::vector<uint8_t> rmOriginal_[2];
    int rmShift_[2]={256,256};
    int rmVertical_[2]={1,1};
    int rmRightVertical_[2]={0,0};
    bool rmAssemblyMode_=false;
    bool rmGeometryDebug_=false;
    stafu::RmCandidatePalette palettes_[3];
    std::string paletteErrors_[3];
    std::vector<uint8_t> standardPalette_;
    std::string standardPaletteError_;
    int rmPaletteMode_=0;
    bool showRm_=false;
    int rmSelected_=0;
    int selected_=0;
};
}
