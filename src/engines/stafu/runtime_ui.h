// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "common/types.h"
#include <functional>
namespace reluva {
class System;
namespace stafu {
class ComputerNavigationIndex;
class ComputerStringIndex;
// Platform adapter: engine stays independent of SDL and ImGui.
using ComputerUiRunner = std::function<Error(System&,const ComputerNavigationIndex&,const ComputerStringIndex&)>;
void setComputerUiRunner(ComputerUiRunner runner);
}
}
