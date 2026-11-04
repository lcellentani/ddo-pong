#include "core/tables.h"
#include "platform/platform.h"
#include "sim/tick.h"

namespace {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr double kTickSeconds = 1.0 / 120.0;

}  // namespace

int main() {
    platform_init(kWindowWidth, kWindowHeight, "ddo-pong");

    Tables tables{};
    while (!platform_poll_input().quit) {
        tick(tables, kTickSeconds);
        platform_draw(tables);
    }

    platform_shutdown();
    return 0;
}
