// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/screen_manager.h"
#include <cassert>
int main() {
    using namespace reluva::stafu;
    ScreenManager screens;
    assert(screens.current()==Screen::MainMenu);
    assert(!screens.show(Screen::MainMenu));
    assert(screens.show(Screen::Computer));
    assert(screens.current()==Screen::Computer);
    assert(screens.show(Screen::BridgePlaceholder));
    assert(screens.current()==Screen::BridgePlaceholder);
    assert(screens.show(Screen::MainMenu));
    assert(screens.current()==Screen::MainMenu);
    return 0;
}
