#include "platform/platform.h"

#include <raylib.h>

#include "core/tables.h"

namespace {

constexpr int kTargetFps = 60;

}  // namespace

void platform_init(int w, int h, const char* title) {
    InitWindow(w, h, title);
    SetTargetFPS(kTargetFps);
}

InputState platform_poll_input() {
    return InputState{.p1_axis = 0.0f, .p2_axis = 0.0f, .quit = WindowShouldClose()};
}

void platform_draw(const Tables& /*t*/) {
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
}

double platform_time_seconds() {
    return GetTime();
}

void platform_shutdown() {
    CloseWindow();
}
