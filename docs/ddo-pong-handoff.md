# ddo-pong — Handoff for Claude Code CLI

> Read this file first. Then read `CLAUDE.md` (if present), then start at **Milestone 0**.
> Items marked **[VERIFY]** are assumptions that the human (Ludo) must confirm or that you must check on the machine before relying on them.

## 1. Purpose

A deliberately small Pong, built as a **learning vehicle for Data-Oriented Design** (Richard Fabian, *Data-Oriented Design*, ISBN 9781916478701).
Pong is too small to show performance wins. The goal is the *discipline*: tables, existence-based processing, transform pipelines, no hidden coupling.

**Success =** a playable 2-player/vs-AI Pong whose code reads as a set of tables and pure transforms, with the full toolchain (build, LSP, lint, format) working from day one.

Owner profile: Technical Director on a AAA engine, 25+ years C++. Skip basics. Be direct, explain trade-offs, push back when something is wrong.

## 2. Design rules (non-negotiable)

1. **Data first.** No `Ball`, `Paddle`, `Game` classes. State lives in tables (struct-of-arrays) keyed by `EntityId`.
2. **Existence-based processing.** State is *membership in a table*, not a flag. No `isAI`, `isActive`, `isDead` bools. Use separate tables (`HumanControlled`, `AIControlled`, `Respawning`, `Moving`…).
3. **Systems are transforms.** Free functions: `(const Tables in…, Tables& out…, dt)`. No hidden state, no globals, no virtuals, no inheritance.
4. **Events are tables.** Collisions, goals and scoring produce rows in an event table consumed by a later system. No callbacks.
5. **Layout follows access.** If a system touches `x,y,dx,dy`, those are contiguous arrays. Keep ids parallel across columns of the same table.
6. **Simulation is separate from presentation.** Fixed timestep (e.g. 120 Hz) for sim; render reads tables and draws. The sim must run headless (needed for tests).
7. **The platform library lives behind one seam** (`platform/` only). No `raylib.h` / `SDL.h` includes anywhere else.
8. Every system gets a unit test that builds tables by hand, runs the transform, and asserts on output tables.

Planned tables (starting point, refine as you go):

| Table | Columns |
|---|---|
| `Position` | id, x, y |
| `Velocity` | id, dx, dy |
| `Extent` | id, half_w, half_h |
| `Paddle`, `Ball`, `Wall` | id (membership only) |
| `HumanControlled` | id, player_slot |
| `AIControlled` | id, reaction_delay, error |
| `PaddleIntent` | id, axis (-1/0/+1) |
| `CollisionEvent` | a, b, normal_x, normal_y |
| `GoalEvent` | scoring_player |
| `Score` | player_slot, points |
| `Respawning` | id, timer |

Pipeline per tick:
`Input → PaddleIntent → Velocity(paddle) → integrate Position → detect CollisionEvent → resolve (bounce) → detect GoalEvent → Score/Respawning → (render reads tables)`

## 3. Toolchain

| Tool | Role | Notes |
|---|---|---|
| **clang / clang++** | Compiler | C++20 minimum. Use a recent stable LLVM (≥ 18) **[VERIFY version installed]** |
| **CMake** | Build generation | ≥ 3.25 for presets v6 |
| **Ninja** | Build backend | Single generator everywhere |
| **clangd** | LSP | Reads `compile_commands.json`; must be the same LLVM major as clang |
| **clang-tidy** | Static analysis | Runs inside clangd and via CMake target |
| **clang-format** | Formatting | Config in `.clang-format` |
| **Zed** | Editor | Uses clangd + tasks from `.zed/` |
| **kitbash** | Claude Code plugins/skills | See section 8 |

**Platform [VERIFY]:** the owner's earlier project (pacman-cpp) targeted Windows/MSVC. Confirm the OS before installing anything. Notes per OS:

- **Windows:** install LLVM (official installer or `winget install LLVM.LLVM`), `winget install Kitware.CMake Ninja-build.Ninja`. Clang on Windows still needs the MSVC STL + Windows SDK (Visual Studio Build Tools, "Desktop development with C++"). Run builds from a shell where the VS environment is loaded (or use `clang-cl` + `Ninja` via the "x64 Native Tools" prompt). If environment issues arise, stop and report instead of improvising.
- **macOS:** `brew install llvm cmake ninja`; use Homebrew's LLVM (`$(brew --prefix llvm)/bin`) ahead of Apple clang on PATH so clang, clangd and clang-tidy match.
- **Linux:** `apt install clang clang-tidy clangd clang-format cmake ninja-build` (or the apt.llvm.org packages for a newer major).

**Verification script (run first, paste output back to the human):**

```sh
clang++ --version && cmake --version && ninja --version \
  && clangd --version && clang-tidy --version && clang-format --version
```

## 4. Repository layout

```
ddo-pong/
├─ CMakeLists.txt
├─ CMakePresets.json
├─ .clang-format
├─ .clang-tidy
├─ .clangd
├─ .gitignore                 # build/, .cache/, compile_commands.json symlink
├─ .zed/
│  ├─ settings.json
│  └─ tasks.json
├─ CLAUDE.md                  # short rules for the agent, derived from section 2
├─ HANDOFF.md                 # this file
├─ docs/
│  └─ specs/                  # SPEC-NNN-*.md, agreed before implementation
├─ src/
│  ├─ core/                   # tables, ids, math. No platform includes
│  ├─ sim/                    # systems (pure transforms) + tick pipeline
│  ├─ platform/               # the ONLY place that includes the window/render lib
│  └─ main.cpp
└─ tests/                     # headless tests of systems
```

Workflow: **SPEC-driven.** For each milestone, write a short `docs/specs/SPEC-NNN-*.md` (inputs tables, output tables, invariants, test cases), get the human's OK, then implement.

## 5. Build system

`CMakePresets.json`:

```json
{
  "version": 6,
  "cmakeMinimumRequired": { "major": 3, "minor": 25, "patch": 0 },
  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "cacheVariables": {
        "CMAKE_CXX_COMPILER": "clang++",
        "CMAKE_C_COMPILER": "clang",
        "CMAKE_CXX_STANDARD": "20",
        "CMAKE_CXX_STANDARD_REQUIRED": "ON",
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },
    { "name": "debug",   "inherits": "base", "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug" } },
    { "name": "release", "inherits": "base", "cacheVariables": { "CMAKE_BUILD_TYPE": "Release" } },
    {
      "name": "asan",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_CXX_FLAGS": "-fsanitize=address,undefined -fno-omit-frame-pointer"
      }
    }
  ],
  "buildPresets": [
    { "name": "debug",   "configurePreset": "debug" },
    { "name": "release", "configurePreset": "release" },
    { "name": "asan",    "configurePreset": "asan" }
  ],
  "testPresets": [
    { "name": "debug", "configurePreset": "debug", "output": { "outputOnFailure": true } }
  ]
}
```

`CMakeLists.txt` (skeleton):

```cmake
cmake_minimum_required(VERSION 3.25)
project(ddo_pong LANGUAGES CXX)

option(DDO_TIDY "Run clang-tidy during build" OFF)

add_library(warnings INTERFACE)
target_compile_options(warnings INTERFACE
  -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
  -Wold-style-cast -Wnon-virtual-dtor -Wdouble-promotion)

# Pure simulation: no platform includes allowed here.
add_library(ddo_core STATIC src/sim/tick.cpp)   # grow as needed
target_include_directories(ddo_core PUBLIC src)
target_compile_features(ddo_core PUBLIC cxx_std_20)
target_link_libraries(ddo_core PRIVATE warnings)

# Platform seam: the only target that links the window/render library.
add_library(ddo_platform STATIC src/platform/platform.cpp)
target_link_libraries(ddo_platform PUBLIC ddo_core PRIVATE warnings)

add_executable(ddo_pong src/main.cpp)
target_link_libraries(ddo_pong PRIVATE ddo_core ddo_platform warnings)

if(DDO_TIDY)
  set_target_properties(ddo_core ddo_platform ddo_pong PROPERTIES
    CXX_CLANG_TIDY "clang-tidy")
endif()

enable_testing()
add_subdirectory(tests)
```

Dependencies: use `FetchContent` with a **pinned tag** (see section 9). Keep the dependency link on `ddo_platform` only.

Daily commands:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
ln -sf build/debug/compile_commands.json compile_commands.json   # or rely on .clangd
```

## 6. Language tooling configs

`.clangd`:

```yaml
CompileFlags:
  CompilationDatabase: build/debug
Diagnostics:
  UnusedIncludes: Strict
  ClangTidy:
    FastCheckFilter: Strict
Index:
  Background: Build
InlayHints:
  Enabled: true
  ParameterNames: true
  DeducedTypes: true
```

`.clang-tidy` (start strict-ish, relax only with a reason written in a comment):

```yaml
Checks: >
  -*,
  bugprone-*,
  performance-*,
  modernize-*,
  readability-*,
  misc-*,
  cppcoreguidelines-init-variables,
  cppcoreguidelines-slicing,
  cppcoreguidelines-narrowing-conversions,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers,
  -readability-identifier-length,
  -misc-non-private-member-variables-in-classes
WarningsAsErrors: 'bugprone-*'
HeaderFilterRegex: '^.*/src/.*'
FormatStyle: file
```

`.clang-format` (adjust to the owner's taste; confirm with him once):

```yaml
BasedOnStyle: LLVM
IndentWidth: 4
ColumnLimit: 110
AllowShortFunctionsOnASingleLine: Inline
PointerAlignment: Left
SortIncludes: CaseSensitive
```

## 7. Zed

`.zed/settings.json`:

```json
{
  "lsp": {
    "clangd": {
      "binary": {
        "path": "clangd",
        "arguments": [
          "--background-index",
          "--clang-tidy",
          "--header-insertion=iwyu",
          "--completion-style=detailed",
          "--compile-commands-dir=build/debug"
        ]
      }
    }
  },
  "languages": {
    "C++": {
      "tab_size": 4,
      "format_on_save": "on",
      "formatter": "language_server"
    }
  }
}
```

`.zed/tasks.json`:

```json
[
  { "label": "configure (debug)", "command": "cmake --preset debug" },
  { "label": "build (debug)",     "command": "cmake --build --preset debug" },
  { "label": "test (debug)",      "command": "ctest --preset debug" },
  { "label": "run",               "command": "./build/debug/ddo_pong" },
  { "label": "tidy (full)",       "command": "cmake --preset debug -DDDO_TIDY=ON && cmake --build --preset debug" }
]
```

Checks after setup: open `src/main.cpp` in Zed, confirm go-to-definition, diagnostics from clang-tidy, and format-on-save all work. If clangd shows "no compile commands", the configure step has not run or `--compile-commands-dir` is wrong.

## 8. kitbash (Claude Code plugins)

kitbash is the owner's personal plugin monorepo (skills, commands, rules), distributed through the Claude Code plugin marketplace.

- **[VERIFY]** exact marketplace id and plugin names with the human before installing. Do not guess.
- Install pattern: `/plugin marketplace add <owner>/kitbash`, then `/plugin install <plugin>@kitbash`.
- Known components: `quartermaster-ledger` (token consumption reporting) and `quartermaster-crew` (model-routed subagent delegation). A code-style/craftsmanship skill is planned but may not exist yet.
- Project-specific rules (section 2) go in `CLAUDE.md`, **not** in kitbash. kitbash stays project-agnostic.
- If a kitbash skill conflicts with this file, this file wins for this repo; flag the conflict to the human.

## 9. Platform library decision: Raylib (default) behind a seam

Default: **Raylib**, wrapped in `src/platform/` with a tiny interface:

```cpp
// platform.h — no library types leak out of this header
struct InputState { float p1_axis; float p2_axis; bool quit; };
void platform_init(int w, int h, const char* title);
InputState platform_poll_input();
void platform_draw(const Tables& t);   // reads tables, draws rects + score
double platform_time_seconds();
void platform_shutdown();
```

Why: for Pong the point is the data design, and Raylib has the shortest path to pixels, input and text with no boilerplate. Because of the seam, switching to SDL3 later is a one-file change and makes a nice follow-up exercise.

Pin the dependency with `FetchContent` to a specific stable release tag **[VERIFY latest stable tag]**; do not track `master`.

Caveat to respect: Raylib keeps global state and owns the window loop. Drive it from `main.cpp` (poll → sim ticks → draw); never let it call into sim code.

## 9b. Milestones

- **M0 – Toolchain:** run the verification script, create presets/CMake skeleton, empty window, clangd + clang-tidy + format-on-save verified in Zed. Commit.
- **M1 – Tables & ids:** `EntityId`, a minimal `Table<...>` SoA helper (or plain structs of vectors; justify the choice in the SPEC), spawn the 2 paddles, ball, walls.
- **M2 – Movement:** input → intent → velocity → integrate. Fixed timestep with accumulator. Unit tests headless.
- **M3 – Collision & events:** `CollisionEvent` table, bounce resolution, tests for corner cases (tunnelling at high speed: decide on swept test vs substeps in a SPEC).
- **M4 – Scoring & respawn:** `GoalEvent`, `Score`, existence-based `Respawning`.
- **M5 – AI paddle:** `AIControlled` table, a separate system; zero branching on "is AI" in other systems.
- **M6 – Stress experiment:** spawn N balls (1k → 100k) to see what the layout buys. Report timings (release build) in `docs/specs/`.

## 10. Definition of done (every milestone)

- Builds clean with warnings on, in `debug` and `release`.
- `ctest` green, also under the `asan` preset.
- clang-tidy: no new warnings (run with `-DDDO_TIDY=ON`).
- clang-format applied.
- SPEC updated; a short note on what the DOD design bought or cost in this milestone.
- Small commits with clear messages.

## 11. Open items for the human

1. Operating system and shell (drives section 3).
2. LLVM major version installed.
3. kitbash marketplace id / plugins to enable for this repo.
4. Raylib vs SDL3 final call (default above is Raylib).
5. Tab/column/brace style preferences for `.clang-format`.
