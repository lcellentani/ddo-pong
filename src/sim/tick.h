#pragma once

#include "core/tables.h"

// Advances the simulation by one fixed step of dt seconds.
void tick(Tables& tables, double dt);
