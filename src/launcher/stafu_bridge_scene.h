// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Milestone 56: SDL3 bridge presentation adapter. No gameplay or asset mutations.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/spr_gif_rgba.h"
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
    }
    ~StafuBridgeScene() {
        for(auto& item:images_)if(item.texture)SDL_DestroyTexture(item.texture);
    }
    StafuBridgeScene(const StafuBridgeScene&)=delete;
    StafuBridgeScene& operator=(const StafuBridgeScene&)=delete;

    void draw() {
        ImGui::TextUnformatted("Bridge scene - resource presentation foundation");
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
    SDL_Renderer* renderer_;
    Image images_[2];
    int selected_=0;
};
}
