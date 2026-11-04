# M0 Toolchain Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A repo that configures, builds, tests and lints from a clean clone on this Windows machine, opens an empty Raylib window, and has clangd, clang-tidy and format-on-save working in Zed.

**Architecture:** Follows handoff sections 4-7 and 9, with the Windows corrections listed below. `ddo_core` stays headless, and only `ddo_platform` links Raylib.

**Tech Stack:** clang++ 22.1.8 (GNU-style driver, `x86_64-pc-windows-msvc`), CMake 4.4.2, Ninja 1.13.2, Raylib 6.2, Zed 1.27.0.

**Spec:** `docs/ddo-pong-handoff.md` (sections 2-7, 9, 9b, 10)

## Machine baseline (measured 2026-11-04)

| Item | Result |
|---|---|
| clang++, clang-cl, clangd, clang-tidy, clang-format | 22.1.8, same major, in `C:\Program Files\LLVM\bin` |
| CMake / Ninja / git / Zed | 4.4.2 / 1.13.2 / 2.55.0 / 1.27.0 |
| MSVC | VS Build Tools 2022 and 2026 both present; clang auto-selects 2026 (MSVC 14.51) |
| Windows SDK | 10.0.26100.0 |
| VS environment needed? | No. Configure, build and run all worked from a plain shell |
| Raylib | 6.2 is the latest stable release (2026-10-15); builds and links with this toolchain |
| kitbash | Marketplace `lcellentani/kitbash`. Installed: `core`, `quartermaster-ledger`, `spec-plan-workflow`, `cpp-engine-conventions`. `quartermaster-crew` is not published there |
| Developer Mode | Not enabled, so `ln -s` style symlinks are unavailable |

No installs are needed.

## Corrections to the handoff

| # | Handoff says | Problem | This plan does |
|---|---|---|---|
| 1 | `asan` preset is plain Debug | **Measured:** the debug CRT (`ucrtbased`) makes ASan abort with a false "attempting free on address which was not malloc()-ed" | Adds `CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`. **Measured:** a real heap overflow is then reported with file and line |
| 2 | No test preset for asan | Section 10 requires ctest under asan, and the runtime DLL is not on PATH (**measured:** the exe fails to load) | Adds an `asan` test preset that puts the runtime dir on PATH |
| 3 | `project(... LANGUAGES CXX)` | Raylib is C; the verified build enabled both | `LANGUAGES C CXX` |
| 4 | `tidy` task passes `-DDDO_TIDY=ON` to the debug preset | The value sticks in the cache, so every later debug build runs tidy | Separate `tidy` preset in `build/tidy` |
| 5 | Tidy task chains with `&&` | Not valid in Windows PowerShell 5.1, if that is Zed's shell | `cmake --workflow --preset tidy`, a single command |
| 6 | `HeaderFilterRegex: '^.*/src/.*'` | May miss backslash paths on Windows (untested; Task 5 checks it) | `'.*[/\\]src[/\\].*'` |
| 7 | `ln -sf` compile_commands | Developer Mode is off, and `.clangd` already points at the database | Dropped |
| 8 | `run` task uses `./build/debug/ddo_pong` | Windows binary name | `build/debug/ddo_pong.exe` |
| 9 | `HANDOFF.md` at repo root | The file is at `docs/ddo-pong-handoff.md` | Leave it there; `CLAUDE.md` links to it |

## Global Constraints

- C++20, CMake >= 3.25, presets version 6, Ninja generator, `clang++`/`clang` drivers (not `clang-cl`, because the warning flags are GNU-style).
- MSVC policy (Ludo, 2026-11-04), same as Flywheel: no `cl.exe`, Visual Studio IDE or `vcvars` shell. VS Build Tools and the Windows SDK are accepted prerequisites, because the `x86_64-pc-windows-msvc` target uses the MSVC STL, CRT and SDK. List both as prerequisites in `SPEC-000` and `CLAUDE.md`.
- No `raylib.h` include outside `src/platform/`. No library types in `platform.h`.
- Raylib is pinned to tag `6.2` through `FetchContent` and linked by `ddo_platform` only.
- No classes, globals, virtuals or inheritance in `src/core` or `src/sim`.
- Commits use `quartermaster-ledger:git-commit`. Work on branch `m0-toolchain`, not `main`.

## Review Focus

1. **Sanitizers silently not running:** a green asan ctest must mean ASan is active. Task 3 proves it with a deliberate overflow.
2. **Seam leak:** `#include <raylib.h>` in `src/sim` must fail to compile, not merely be discouraged. Task 4 checks it.
3. **Tidy blind to headers on Windows paths:** a warning in a `src/` header must be reported. Task 5 checks it.
4. **Clean clone:** everything must work after deleting `build/`, including the Raylib fetch. Task 6 checks it.
5. **Zed started from the Start menu:** clangd and tasks must work without an inherited dev shell. Task 6 checks it.

---

### Task 1: Repo baseline and SPEC-000

**Files:**
- Create: `.gitignore`, `.clang-format`, `CLAUDE.md`, `docs/specs/SPEC-000-toolchain.md`
- Add to git: `docs/ddo-pong-handoff.md`, this plan

- [ ] **Step 1:** Create branch `m0-toolchain` from `main`.
- [ ] **Step 2:** Write `.gitignore` with `build/`, `.cache/`, `compile_commands.json`.
- [ ] **Step 3:** Copy `.clang-format` from the kitbash asset named in Decision 1.
- [ ] **Step 4:** Write `CLAUDE.md`: the section 2 rules in short form, a link to `docs/ddo-pong-handoff.md`, the daily commands, and "the handoff wins over kitbash skills; flag conflicts".
- [ ] **Step 5:** Write `docs/specs/SPEC-000-toolchain.md`: the baseline table, the corrections table, and the Task 6 acceptance checklist.
- [ ] **Step 6:** Commit.

### Task 2: CMake skeleton and headless test

**Files:**
- Create: `CMakeLists.txt`, `CMakePresets.json`, `src/core/tables.h`, `src/sim/tick.h`, `src/sim/tick.cpp`, `src/main.cpp`, `tests/CMakeLists.txt`, `tests/tick_smoke_test.cpp`

**Interfaces:**
- Produces: `struct Tables {};` in `src/core/tables.h` (empty until M1); `void tick(Tables& tables, double dt);` in `src/sim/tick.h` (no-op until M2); CMake targets `ddo_core`, `ddo_pong`, `warnings`.

M0 tests are plain `int main()` executables registered with `add_test`. Choosing a test framework belongs to the M1 SPEC.

- [ ] **Step 1:** Write `tests/tick_smoke_test.cpp`: builds a `Tables`, calls `tick(tables, 1.0 / 120.0)`, returns 0. Register it as `tick_smoke`, linked to `ddo_core` only.
- [ ] **Step 2:** Write `CMakePresets.json` from handoff section 5 without `asan` (Task 3 adds it). Add a `release` test preset. Write `CMakeLists.txt` from section 5 with `LANGUAGES C CXX`, leaving out `ddo_platform` until Task 4. `src/main.cpp` is `int main() { return 0; }` for now.
- [ ] **Step 3:** Run `cmake --preset debug` and `cmake --build --preset debug`. Expected: FAIL, because `tick` is not defined yet.
- [ ] **Step 4:** Add `tick.h`, `tick.cpp` and `tables.h`.
- [ ] **Step 5:** Run the build, then `ctest --preset debug`, then the same for `release`. Expected: zero warnings, `100% tests passed` for both.
- [ ] **Step 6:** Commit.

### Task 3: ASan and UBSan preset

**Files:**
- Modify: `CMakePresets.json`

**Interfaces:**
- Produces: configure, build and test presets named `asan`.

- [ ] **Step 1:** Add the configure preset: `CMAKE_BUILD_TYPE=Debug`, `CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`, `CMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer`.
- [ ] **Step 2:** Add the build preset and a test preset with `"environment": { "PATH": "C:/Program Files/LLVM/lib/clang/22/lib/windows;$penv{PATH}" }`. Add a comment in `SPEC-000` that this path tracks the LLVM major version.
- [ ] **Step 3:** Run `cmake --preset asan`, `cmake --build --preset asan`, `ctest --preset asan`. Expected: `100% tests passed`.
- [ ] **Step 4:** Temporarily add a heap overflow to `tick_smoke_test.cpp`, with an index the optimizer cannot remove. Run ctest. Expected: FAIL with `heap-buffer-overflow`. Repeat with a signed integer overflow. Expected: a UBSan `runtime error` line. UBSan detection has not been measured yet, so if nothing is reported, stop and report.
- [ ] **Step 5:** Revert the test, confirm green, commit.

### Task 4: Raylib behind the seam, empty window

**Files:**
- Create: `src/platform/platform.h`, `src/platform/platform.cpp`
- Modify: `CMakeLists.txt`, `src/main.cpp`

**Interfaces:**
- Consumes: `Tables`, `tick` from Task 2.
- Produces: the five `platform_*` functions and `InputState`, exactly as in handoff section 9. In M0, axes are `0.0f`, `quit` is `WindowShouldClose()`, and `platform_draw` clears the background.

- [ ] **Step 1:** Add `FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 6.2 GIT_SHALLOW TRUE SYSTEM)`. Set `BUILD_EXAMPLES` and `BUILD_GAMES` to `OFF`. `SYSTEM` keeps our warnings off `raylib.h`.
- [ ] **Step 2:** Add `ddo_platform` as in section 5, with `raylib` linked `PRIVATE`.
- [ ] **Step 3:** Implement `platform.cpp`. `main.cpp` runs the poll, tick, draw loop until `quit`. Raylib never calls into sim code.
- [ ] **Step 4:** Build `debug`, `release` and `asan`. Expected: no warnings from our targets.
- [ ] **Step 5:** Run `build/debug/ddo_pong.exe`. Expected: a window opens and the process exits with 0 when closed.
- [ ] **Step 6:** Temporarily add `#include <raylib.h>` to `src/sim/tick.cpp`, then to `src/main.cpp`. Expected: both fail with "file not found". Revert.
- [ ] **Step 7:** Commit.

### Task 5: clang-tidy and clangd

**Files:**
- Create: `.clang-tidy`, `.clangd`
- Modify: `CMakePresets.json`

**Interfaces:**
- Produces: configure and build presets named `tidy`, plus a workflow preset `tidy` (configure, then build).

- [ ] **Step 1:** Write `.clangd` and `.clang-tidy` from handoff section 6, using correction 6 for `HeaderFilterRegex`.
- [ ] **Step 2:** Add the `tidy` presets with `DDO_TIDY=ON` and their own binary dir, `build/tidy`.
- [ ] **Step 3:** Run `cmake --workflow --preset tidy`. Fix what it reports, or disable a check with a written reason, per section 6. Expected: no warnings.
- [ ] **Step 4:** Temporarily add an uninitialized local in an inline function in `src/sim/tick.h`. Run the workflow. Expected: a warning with the header's path. Revert.
- [ ] **Step 5:** Run `cmake --build --preset debug` after touching a source file. Expected: no clang-tidy invocation.
- [ ] **Step 6:** Run `clangd --check=src/main.cpp`. Expected: finds `build/debug/compile_commands.json`, no errors.
- [ ] **Step 7:** Commit.

### Task 6: Zed and M0 acceptance

**Files:**
- Create: `.zed/settings.json`, `.zed/tasks.json`
- Modify: `docs/specs/SPEC-000-toolchain.md`

- [ ] **Step 1:** Write `.zed/settings.json` from handoff section 7. Write `.zed/tasks.json` from section 7 with corrections 5 and 8, adding a `test (asan)` task.
- [ ] **Step 2 (Ludo, manual):** Start Zed from the Start menu and open the repo. In `src/main.cpp`, confirm go-to-definition into `platform.h`, a clang-tidy diagnostic on a temporary issue, and format-on-save. Run each task from the task picker.
- [ ] **Step 3:** Run `clang-format --dry-run --Werror` on all files in `src/` and `tests/`. Expected: no output.
- [ ] **Step 4:** Delete `build/`. Run the configure, build and test sequence for `debug`, `release` and `asan`, then the `tidy` workflow. Expected: everything green with no warnings from our targets.
- [ ] **Step 5:** Add the "what DOD bought or cost" note to `SPEC-000`. Commit.

## Decisions (resolved 2026-11-04)

1. **Formatting (handoff open item 5):** use the kitbash asset from `cpp-engine-conventions` 0.5.0 (`skills/clang-format/assets/.clang-format`): Google base, 4 spaces, 120 columns. This replaces the handoff section 6 config.
2. **Raylib (open item 4):** Raylib 6.2.
3. **kitbash (open item 3):** marketplace `lcellentani/kitbash`. All four plugins it publishes are installed, so nothing to install. `quartermaster-crew`, named in handoff section 8, is not published there.
4. **Execution:** Native.
