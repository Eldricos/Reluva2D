// SPDX-License-Identifier: GPL-3.0-or-later
#include "backends/sdl3/sdl_system.h"
#include "engines/stafu/runtime_ui.h"
#include "engines/stafu/computer_browser.h"
#include "engines/stafu/computer_string_index.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <string>
namespace reluva {
namespace {
std::string textField(const stafu::ComputerStringIndex::Field& f) {
    using Index=stafu::ComputerStringIndex;
    if(f.status==Index::Status::Empty)return {};
    if(f.status!=Index::Status::Text)return std::string("[")+Index::statusName(f.status)+"]";
    std::string result;
    for(size_t i=0;i<f.text.size();++i) {
        if(f.text[i]=='\r') {
            result+='\n';
            if(i+1<f.text.size()&&f.text[i+1]=='\n')++i;
        } else result+=f.text[i];
    }
    return result;
}
Error runComputerUi(System& system,const stafu::ComputerNavigationIndex& graph,
                    const stafu::ComputerStringIndex& strings) {
    auto* sdl=dynamic_cast<SdlSystem*>(&system);
    if(!sdl)return Error::fail("STAFU requires the SDL3 graphical backend.");
    stafu::ComputerBrowser browser(graph);
    SDL_Renderer* renderer=sdl->renderer();
    sdl->setWindowTitle("STAFU - Ship's Computer");
    bool running=true;
    while(running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if(event.type==SDL_EVENT_QUIT) {
                // Preserve the global quit request through SdlSystem.
                SDL_PushEvent(&event);
                reluva::Event translated;
                sdl->pollEvent(translated);
                running=false;
            } else if(event.type==SDL_EVENT_KEY_DOWN &&
                      event.key.key==SDLK_F5 && (event.key.mod&SDL_KMOD_CTRL)) {
                running=false;
            }
        }
        if(!running)break;
        if(SDL_GetWindowFlags(sdl->window())&SDL_WINDOW_MINIMIZED) {
            SDL_Delay(16);continue;
        }
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        const auto* vp=ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::Begin("STAFU Runtime",nullptr,
            ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|
            ImGuiWindowFlags_NoSavedSettings);
        if(ImGui::Button("Return to Launcher"))running=false;
        ImGui::SameLine();
        ImGui::BeginDisabled(browser.depth()<2);
        if(ImGui::Button("Back"))browser.back();
        ImGui::EndDisabled();
        ImGui::SameLine();
        if(ImGui::Button("Home"))browser.home();
        ImGui::Separator();
        const auto& node=graph.nodes()[browser.current()];
        const auto& entry=strings.entries()[browser.current()];
        const float width=ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("Topics",ImVec2(width*0.32f,0),ImGuiChildFlags_Borders);
        ImGui::TextUnformatted("Ship's Computer");
        ImGui::Separator();
        for(size_t i=0;i<node.children.size();++i) {
            const auto label=textField(strings.entries()[node.children[i]].fields[0]);
            ImGui::PushID(static_cast<int>(i));
            if(ImGui::Selectable(label.empty()?"(untitled)":label.c_str()))
                browser.select(i+1);
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("Article",ImVec2(0,0),ImGuiChildFlags_Borders);
        const auto title=textField(entry.fields[1]);
        const auto fallback=textField(entry.fields[0]);
        ImGui::TextWrapped("%s",title.empty()?fallback.c_str():title.c_str());
        ImGui::Separator();
        const auto article=textField(entry.fields[2]);
        if(article.empty())ImGui::TextDisabled("No article text.");
        else ImGui::TextUnformatted(article.c_str());
        ImGui::EndChild();
        ImGui::End();
        ImGui::Render();
        const auto& io=ImGui::GetIO();
        SDL_SetRenderScale(renderer,io.DisplayFramebufferScale.x,io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer,20,22,28,255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),renderer);
        SDL_RenderPresent(renderer);
    }
    SDL_SetRenderScale(renderer,1.0f,1.0f);
    return Error::success();
}
}
void registerStafuRuntimeUi() {stafu::setComputerUiRunner(runComputerUi);}
}
