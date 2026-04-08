# Hierarchical Island & Grid System

> Planned for Days 7–8. Current islands are hand-placed `BP_GridCube` actors (Path A). This system replaces that once the core slice is complete.

---

## Objective

Refactor world generation from a single global manager to a modular, instance-based system where `ATaeIsland` actors manage local grid spawning, state logic, and visual styling.

---

## Core Components

### `ATaeIsland` — The Controller

Replaces `ATaeGridManager`. One actor per island, placed in the level.

- **Local Spawning** — spawns an N×M×K grid of `ATaeGridCube` actors relative to its own transform
- **Archetype Binding** — holds a reference to a row in `DT_IslandArchetypes` to determine layout and decay level
- **State Propagation** — owns a single `UTaeStateComponent` that listens for `Arcane.Vision` and broadcasts the state change to all child cubes directly; child cubes no longer register with GAS themselves
- **Randomisation** — uses a per-instance `FRandomStream` seed (exposed as `EditAnywhere`) so procedural variation is consistent across play sessions

> **Concern:** `ATaeGridCube` currently registers its own ASC listener in `BeginPlay`. When `ATaeIsland` takes ownership of state propagation, GridCube's `BeginPlay` ASC registration must be removed — otherwise both fire and cubes react twice. The interface should change: GridCube exposes a public `SetArcaneState(bool)` method; Island calls it directly.

### `FTaeIslandArchetype` — The Data

A `USTRUCT` used as a DataTable row to define island templates.

| Field | Type | Description |
|-------|------|-------------|
| `GridDimensions` | `FIntVector` | Bounds of the island (X × Y × Z cubes) |
| `DecayFactor` | `float` (0–1) | Probability a cube slot spawns junk or nothing instead of a grid cube |
| `bIsPuzzleIsland` | `bool` | If true, use a fixed layout DataTable; if false, use procedural noise + DecayFactor |

> **Concern:** `bIsPuzzleIsland` implies a second DataTable for fixed layouts (e.g. `DT_IslandLayout`). The naming distinction between `DT_IslandArchetypes` (what kind of island) and `DT_IslandLayout` (exact cube positions) should be made explicit before implementation to avoid confusion.

---

## Procedural Logic — "Factory Decay" Styling

During the spawn loop, each grid slot rolls against `DecayFactor` using the island's `FRandomStream`:

```
if (RandomStream.FRand() > DecayFactor)  →  spawn BP_GridCube
else                                      →  spawn BP_IndustrialJunk  OR  skip slot
```

> **Concern:** `BP_IndustrialJunk` is a new actor type that adds scope to Day 7. Recommend deferring it to Day 8 or later so it doesn't block the Island refactor. The skip-slot path alone gives enough visual variety for the initial pass.

### Material Layering

Island passes its archetype's materials to each spawned GridCube at spawn time. GridCube already has `SetMaterial` logic — Island is responsible for configuration, GridCube for application.

> **Concern:** The original proposal had Island applying materials to children after spawn. Passing materials at spawn time is cleaner — no post-spawn iteration needed, and GridCube remains self-contained.

---

## Integration

### GAS
`GA_SpectralShift` continues to toggle the global `Arcane.Vision` tag on the player. No changes needed to the ability system.

### MVVM
`UTaeHudViewModel` remains agnostic of world structure — it only reports whether vision is active. Islands read from `TAG_Arcane_Vision` via their `UTaeStateComponent`, same as current `ATaeGridCube` behaviour.

---

## Day 8 Addition — `ATaeWorldManager`

Not in the original proposal — added based on the game's root-connection mechanic.

One `ATaeWorldManager` per level:
- Holds references to all `ATaeIsland` actors
- Stores hidden connection path data between islands (the routes Ant can grow roots along)
- On `Arcane.Vision` active: reveals connection paths (visibility + collision on)
- On `Arcane.Vision` inactive: hides them again

> **Concern:** Connection paths need a representation — likely `ATaeConnectionPath` actors (spline or point-to-point mesh) placed by a designer and registered with `ATaeWorldManager`. The exact data structure (array of pairs, graph, etc.) needs a decision before implementation.

---

## Proposed Class Hierarchy

```
ATaeWorldManager        (one per level)
  └── TArray<ATaeIsland*>  SpawnedIslands
  └── TArray<ATaeConnectionPath*>  HiddenPaths

ATaeIsland              (one per island, replaces ATaeGridManager)
  └── UTaeStateComponent   — single GAS listener per island
  └── TArray<ATaeGridCube*>  SpawnedCubes
  └── FTaeIslandArchetype  ArchetypeData

ATaeGridCube            (unchanged public interface)
  └── SetArcaneState(bool)  — called by Island, no GAS dependency
```
