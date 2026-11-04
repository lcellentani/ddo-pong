#pragma once

#include "core/tables.h"

struct InputState {
    float p1_axis;
    float p2_axis;
    bool quit;
};

void platform_init(int w, int h, const char* title);
InputState platform_poll_input();
void platform_draw(const Tables& t);
double platform_time_seconds();
void platform_shutdown();
