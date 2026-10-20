// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
namespace reluva::stafu {
// Runtime screens are explicit states; transitions are validated centrally.
enum class Screen { MainMenu, Computer, BridgePlaceholder };
class ScreenManager {
public:
    Screen current() const { return current_; }
    bool show(Screen next) {
        if(next==current_)return false;
        current_=next;
        return true;
    }
private:
    Screen current_=Screen::MainMenu;
};
}
