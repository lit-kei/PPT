#pragma once

#include "resource.h"
#include <tuple>

#define CLIENT_LEFT 700
#define CLIENT_TOP 10
#define CLIENT_WIDTH 640
#define CLIENT_HEIGHT 360

struct Data {
    std::tuple<int, int, int> shaft; // x, y, color
    std::tuple<int, int, int> child;
    int display; // 0 - ˜gü, 1 - ”¼“§–¾‚Ì“h‚è‚Â‚Ô‚µ
    Data(std::tuple<int, int, int> shaft, std::tuple<int, int, int> child, int display) : shaft(shaft), child(child), display(display) {}
};