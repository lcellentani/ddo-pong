# ddo-pong

A small Pong built to practise Data-Oriented Design. Full context: `docs/ddo-pong-handoff.md`. Milestone specs: `docs/specs/`. Write and agree a SPEC before implementing a milestone.

## Design rules (non-negotiable)

1. **Data first.** No `Ball`, `Paddle` or `Game` classes. State lives in struct-of-arrays tables keyed by `EntityId`.
2. **Existence-based processing.** State is table membership, never a flag: no `isAI`, `isActive`, `isDead`.
3. **Systems are transforms.** Free functions `(const Tables in..., Tables& out..., dt)`. No hidden state, globals, virtuals or inheritance.
4. **Events are tables.** Collisions, goals and scoring write rows that a later system consumes. No callbacks.
5. **Layout follows access.** Fields a system touches together are contiguous arrays, with ids parallel across a table's columns.
6. **Simulation is separate from presentation.** Fixed timestep; render only reads tables; the sim runs headless.
7. **One platform seam.** `raylib.h` is included only under `src/platform/`, and no library types leave `platform.h`.
8. **Every system has a unit test** that builds tables by hand, runs the transform and asserts on the output tables.

## Toolchain

Windows, `clang++`/`clang` 22 with CMake presets and Ninja, targeting `x86_64-pc-windows-msvc`.

MSVC policy: no `cl.exe`, Visual Studio IDE or `vcvars` shell. VS Build Tools and the Windows SDK are required prerequisites, because this target uses the MSVC STL, CRT and SDK. Clang finds them on its own from a plain shell.

## Daily commands

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Presets: `debug`, `release`, `asan`. Definition of done for a milestone: handoff section 10.

## Precedence

The handoff and this file win over kitbash skills. Flag any conflict to Ludo. Known conflicts with `cpp-engine-conventions`:

- It prefers C++ modules (`.ixx`). This repo uses headers, as in handoff section 9.
- It gives member variables a trailing underscore. Table columns are plain public data with no underscore (`half_w`), as in handoff section 2.
