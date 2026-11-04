#include "core/tables.h"
#include "sim/tick.h"

int main() {
    Tables tables{};
    tick(tables, 1.0 / 120.0);
    return 0;
}
