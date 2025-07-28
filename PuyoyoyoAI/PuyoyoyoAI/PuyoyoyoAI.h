#pragma once

#include "resource.h"
#include <tuple>
#include <vector>

#define CLIENT_LEFT 700
#define CLIENT_TOP 10
#define CLIENT_WIDTH 640
#define CLIENT_HEIGHT 360


#define ASSIST_WIDTH 480
#define ASSIST_HEIGHT 600

struct Data {
    std::vector<std::tuple<int, int, int>> puyos; // x, y, color
    int display; // 0 - ˜gü, 1 - ”¼“§–¾‚Ì“h‚è‚Â‚Ô‚µ
    Data(std::vector<std::tuple<int, int, int>> puyos, int display) : puyos(puyos), display(display) {}
};