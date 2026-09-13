# AGENTS.md

## Project Overview

C++20 MUD game "Carrot Valley" (MudGame). MVC architecture. CMake min 4.0; MSVC is the primary toolchain. Source comments are Chinese UTF-8.

## Build & Test

```bash
cmake -B cmake-build-debug -S .
cmake --build cmake-build-debug
```

Run all tests:
```bash
cd cmake-build-debug && ctest --output-on-failure
```

Run a single test (example):
```bash
cd cmake-build-debug && ctest -R mining_core_flow_test --output-on-failure
```

Test target naming is predictable: `cmdparser_*_test`, `timeservice_*_test`, `mining_*_test`, `view_panel_test` — use `-R <name>` to filter.

GoogleTest v1.15.2 (via FetchContent). Tests live in `tests/` with one `CMakeLists.txt` per module (cmdparser, timeservice, mining_controller, view).

## Architecture

**Read `docs/MvcGuideline.md`** — it is the authoritative architecture doc.

Key rules:
- **Controller** → Model (read/write), Controller → View (call render)
- **Model** → nothing (pure data + logic, no UI/IO)
- **View** → nothing (pure render, receives data via parameters)
- Use **constructor injection** to reference other systems from controllers
- Maintain strict public/private boundaries

### Build architecture (two tiers — non-obvious)

- **Standalone libs**: aggregated in `src/CMakeLists.txt` via `add_subdirectory` — `time_service`, `model_objects`, `bag`, `map`, `playerstates`, `food`, `crop`, `fertilizer`, `farm`, `game`, `mining_controller`, `player`, `player_serializer` (src/Tool), `tool_controller`, `weather_controller`, `audio` (src/Audio, MCI looping BGM on `winmm`), view libs, `cmd_parser`.
- **Jerry modules**: Food, Crop, Fertilizer, Farm, Fish, Farming, Fishing, Market live under `src/controller/` but are **not** separate library targets — their sources are listed in `JERRY_SOURCES` in the root `CMakeLists.txt` and compiled directly into the `MudGame` executable. `main.cpp` is the composition root (constructor-wires every subsystem, REPL loop + background time thread guarded by a `worldMutex`).

This is why the root `CMakeLists.txt` adds their `include/` dirs to the executable target, and why the executable target must link the other libs in that specific list.

## Directory Layout

```
src/
  controller/         # Game controllers + logic (NOT a lib)
    Crop/ Farm/ Fertilizer/ Fish/ Food/ Farming/ Fishing/ Market/  # Jerry, in-executable
    Game/             # Main game controller (lib `game`)
    Mining_controller/# Mining system (lib `mining_controller`)
    Player/           # Player controller (lib `player`)
    Tool/             # Tool controller (lib `tool_controller`)
    Weather/          # Weather/event controller (lib `weather_controller`)
  model/              # Data + pure logic (libs)
    Bag/ Map/ Objects/ Playerstates/ timeService/
  View/               # view_primitives, view_panels, cmd_parser
  Tool/               # Player save/load serializer (lib `player_serializer`)
  Data/Ore/           # JSON data: ore.json, mining_layers.json, spawn_rates.json
tests/                # GoogleTest unit tests (per-module dirs)
docs/                 # MvcGuideline.md is canonical
main.cpp              # composition root / game loop (windows-only: includes <windows.h>)
```

## Conventions

- **Headers** go in `include/` subdirectory within each component; **sources** in `src/`
- Each library module has its own `CMakeLists.txt` — add new source files there explicitly (no globbing)
- Follow the coding patterns in MvcGuideline.md (Model: pure logic, View: pure render, Controller: coordination)

## Gotchas

- **Case sensitivity**: on-disk `src/controller`, `src/model`, `src/model/timeService` are lowercase/mixed, but CMake references them as `Controller/`, `Model/`, `Model/Timeservice`. Works on Windows; breaks on case-sensitive filesystems (Linux/macOS).
- **MSVC needs `/utf-8`**: source files contain Chinese UTF-8 comments; without the `/utf-8` flag already set in root `CMakeLists.txt`, MSVC parses them as GBK and errors out (C2447/C4819). If you compile a file standalone, pass `/utf-8`.
- **Repo-root-relative quoted includes**: many sources include headers by repo-root path (e.g. `"Controller/Game/include/Game.h"`, `"Model/Map/include/Home.h"`). `src/CMakeLists.txt` adds `src/` to the global include path for this reason.
- **`MUDGAME_DATA_DIR`**: `mining_controller` PUBLICLY defines this compile definition pointing at `src/Data`; ore/mining tests need it to locate JSON data. New tests linking `mining_controller` get it automatically.
- **pjh_json**: dependency is on `main` branch, built with `PJH_JSON_ENABLE_OPTIMIZATIONS=OFF` to stop its aggressive flags (`-march=native`/`-ffast-math`/`-flto`) from leaking into the whole project.
- **`sub/` at repo root is empty** — likely an abandoned directory; leave it be.