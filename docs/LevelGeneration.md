# Level Generation — Hand-Placed Islands, Clutter, Spawners

> Planned for Days 7–8. Replaces the original `ATaeIsland` / `FTaeIslandArchetype` procedural proposal (DataTable-driven cube grids). Placeholder `BP_GridCube` islands (Path A) are retired in favor of hand-placed rock/ruin set dressing.

---

## Objective

Stop generating islands procedurally. Build them by hand as level art, and reserve procedural systems for what actually benefits from randomisation: small clutter dressing and interactable placement. The root-connection mechanic (Arcane Vision reveals hidden paths between islands) is unchanged — only how islands and their contents come into existence changes.

---

## Core Components

### Islands — Hand-Placed Set Dressing

No actor class. An "island" is just a cluster of manually placed static mesh actors (`BP_Rock_*`, `BP_Ruin_*`) arranged in the level by hand. No spawning code, no archetype DataTable — this is level art authoring.

### `ATaeClutterScatter` — Randomised Small Props

Replaces the `DecayFactor` procedural roll from the old proposal. One instance placed per island (or per region).

- **Bounds** — a box or spline component defining the scatter area
- **Mesh pool** — `TArray<TObjectPtr<UStaticMesh>>` (small rocks, wire piles, scrap/junk — `BP_IndustrialJunk` folds into this pool instead of being a dedicated decay-fail actor)
- **Placement** — on construction (or `BeginPlay`), scatters N instances via `UInstancedStaticMeshComponent`; per-instance random position within bounds, random rotation/uniform scale, optional ground-snap via downward line trace
- **Reproducibility** — per-instance `FRandomStream` seed (`EditAnywhere`) so layout is stable across play sessions until the seed changes

> **Concern:** ISM instances share one component per mesh type — if the pool mixes meshes, the actor needs one `UInstancedStaticMeshComponent` per distinct mesh, not one shared component.

### `ATaeInteractableSpawnPoint` — Designer Markers

Lightweight marker actor (icon-only in editor, no mesh at runtime). Designer places more of these per island than the number of items that will actually spawn — the spawner below decides which ones activate.

### `ATaeInteractableSpawner` — Semi-Random Activation

One per level (or per island, if density needs to vary by region).

- Collects all `ATaeInteractableSpawnPoint`s in range at `BeginPlay`
- Per point, rolls against `EditAnywhere SpawnChance` (`FRandomStream`) — only a subset of markers actually spawn something
- For each point that hits, picks a random interactable class from a weighted pool (`BP_Chest`, `BP_Pickup_Mana`, etc.)

> **Concern:** Weighted random selection needs a small struct (`FTaeInteractableEntry { TSubclassOf<AActor> Class; float Weight; }`) rather than a flat `TArray` of classes, so rarer items (chests) can be weighted lower than common pickups.

---

## Root-Connection Mechanic — Unchanged

### `ATaeWorldManager`

One per level:
- Holds references to `ATaeRootPath` actors (renamed from the original proposal's `ATaeConnectionPath` — "root" matches the game's own vocabulary: Ant grows roots between islands)
- On `Arcane.Vision` active: reveals paths (visibility + collision on)
- On `Arcane.Vision` inactive: hides them again

### `ATaeRootPath`

Spline-based actor representing one hidden root connection between two islands. Placed by a designer, registered with `ATaeWorldManager`. Hidden by default — same reveal pattern `ATaeGridCube` used (`UTaeStateComponent` listening for `Arcane.Vision`), just applied to a spline mesh instead of a cube.

---

## Integration

### GAS
`GA_SpectralShift` continues to toggle the global `Arcane.Vision` tag on the player. No changes needed.

### MVVM
`UTaeHudViewModel` remains agnostic of world structure.

### Retired
`ATaeGridManager`, `FTaeIslandArchetype`, `DT_IslandArchetypes`, the `DecayFactor` roll, and `BP_IndustrialJunk` as a standalone actor are all dropped. `ATaeGridCube`'s reveal pattern (`UTaeStateComponent` → material/collision swap) survives as the template for `ATaeRootPath`'s reveal behaviour.

---

## Proposed Class Hierarchy

```
ATaeWorldManager              (one per level)
  └── TArray<ATaeRootPath*>     HiddenPaths

ATaeClutterScatter             (one per island/region, hand-placed)
  └── TArray<UInstancedStaticMeshComponent*>  per mesh in pool
  └── FRandomStream             Seed

ATaeInteractableSpawnPoint     (marker, many per island, hand-placed)

ATaeInteractableSpawner        (one per level or per island)
  └── TArray<ATaeInteractableSpawnPoint*>  Points
  └── TArray<FTaeInteractableEntry>        WeightedPool

ATaeRootPath                   (one per connection, hand-placed)
  └── UTaeStateComponent         reveal listener (same pattern as ATaeGridCube)
```
