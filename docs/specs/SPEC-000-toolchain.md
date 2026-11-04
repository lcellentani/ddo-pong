# SPEC-000: Toolchain (Milestone 0)

**Status:** Done
**Plan:** `docs/superpowers/plans/2026-11-04-m0-toolchain.md`
**Source:** `docs/ddo-pong-handoff.md`

## Goal

A repo that configures, builds, tests and lints from a clean clone on Windows, opens an empty Raylib window, and has clangd, clang-tidy and format-on-save working in Zed.

M0 has no tables or systems, so `Tables` is empty and `tick` is a no-op until later milestones.

## Prerequisites

| Item | Measured 2026-11-04 |
|---|---|
| LLVM (clang++, clangd, clang-tidy, clang-format) | 22.1.8, all the same major |
| CMake / Ninja / git / Zed | 4.4.2 / 1.13.2 / 2.55.0 / 1.27.0 |
| VS Build Tools | 2022 and 2026 installed; clang selects 2026 (MSVC 14.51) |
| Windows SDK | 10.0.26100.0 |

## Decisions

| Decision | Choice |
|---|---|
| MSVC policy | Same as Flywheel: no `cl.exe`, Visual Studio IDE or `vcvars` shell. VS Build Tools and the Windows SDK are required, because the `x86_64-pc-windows-msvc` target uses the MSVC STL, CRT and SDK |
| Compiler driver | `clang++`/`clang`, because the handoff's warning flags are GNU-style |
| Formatting | kitbash asset from `cpp-engine-conventions` 0.5.0 (Google base, 4 spaces, 120 columns), replacing handoff section 6 |
| Platform library | Raylib 6.2 through `FetchContent` |
| Tests in M0 | Plain executables with `add_test`; the M1 SPEC chooses a framework |

## Deviations from the handoff

| # | Handoff | Change | Reason |
|---|---|---|---|
| 1 | `asan` preset is plain Debug | Add `CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL` | Measured: the debug CRT makes ASan abort with a false "attempting free" error; with this setting a real overflow is reported |
| 2 | No asan test preset | Add one, with the ASan runtime dir on PATH | Section 10 requires it; the exe does not load otherwise. The path contains the LLVM major (`clang/22`) and must change with it |
| 3 | `LANGUAGES CXX` | `LANGUAGES C CXX` | Raylib is C |
| 4 | Tidy through `-DDDO_TIDY=ON` on `debug` | Separate `tidy` preset in `build/tidy` | The cached option would run tidy on every later debug build |
| 5 | Tidy task chained with `&&` | `cmake --workflow --preset tidy` | One command, independent of Zed's shell |
| 6 | `HeaderFilterRegex: '^.*/src/.*'` | `'.*[/\\]src[/\\].*'` | Measured: the original filter reports 0 warnings from a `src/` header on Windows, and this one reports 1 |
| 7 | `ln -sf` compile_commands | Dropped | Developer Mode is off, and `.clangd` points at the database |
| 8 | `./build/debug/ddo_pong` | `build/debug/ddo_pong.exe` | Windows binary name |
| 9 | `HANDOFF.md` at the repo root | Stays at `docs/ddo-pong-handoff.md` | Avoids moving the file; `CLAUDE.md` links to it |
| 10 | `asan` flags have no recover setting | Add `-fno-sanitize-recover=all` | Measured: UBSan otherwise prints the error and exits 0, so `ctest` passes on real undefined behaviour |
| 11 | `.clang-tidy` check list | Disable `readability-uppercase-literal-suffix` | It demands `0.0F`, but the project style writes `0.016f` |
| 12 | Test presets have no `noTestsAction` | Set it to `error` in a hidden base test preset | Measured: `ctest` exits 0 with "No tests were found" on an unconfigured tree |
| 13 | `CXX_CLANG_TIDY` is plain `clang-tidy` | Add `--warnings-as-errors=*` in CMake only | Measured: a warning printed once, then a rerun was clean with the bug still there. `.clang-tidy` is unchanged, so Zed still shows warnings |
| 14 | Nothing proves sanitizers stay on | Add `asan_canary` and `ubsan_canary` tests to the `asan` preset | Measured: they fail if the sanitizer flags or `-fno-sanitize-recover=all` are removed |

## Acceptance

- [x] Deleting `build/`, then configure, build and `ctest` pass for `debug`, `release` and `asan`, with no warnings from our targets.
- [x] `ctest --preset asan` fails on a deliberate heap overflow and on a deliberate signed overflow.
- [x] `cmake --workflow --preset tidy` reports no warnings, and does report a deliberate warning in a `src/` header.
- [x] A normal debug build does not run clang-tidy.
- [x] `build/debug/ddo_pong.exe` opens a window and exits with 0 when closed.
- [x] `#include <raylib.h>` fails to compile in `src/sim/` and `src/main.cpp`.
- [x] clangd finds the compilation database and publishes clang-tidy diagnostics over LSP. `clangd --check` alone cannot show this, because it only prints errors.
- [x] Zed started from the Start menu: go-to-definition, clang-tidy diagnostics and format-on-save work, and every task runs.
- [x] `clang-format --dry-run --Werror` is clean on `src/` and `tests/`.

## What DOD bought or cost

M0 has no tables or systems, so this is about structure, not data layout.

**Bought:** the sim builds and tests headless, with no Raylib in its link line. The seam is enforced by the build: `raylib.h` fails to compile outside `src/platform/`, so rule 7 needs no review to hold.

**Cost:** one extra library target, and placeholder `Tables` and `tick` that do nothing yet. `tick_smoke` proves the wiring only.
