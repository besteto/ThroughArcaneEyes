# Development Roadmap — Through Arcane Eyes

> For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

### Sprint 1 — Core Vertical Slice
*Playable portfolio demo: character, Arcane mechanic, grid world, UI, portal.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 1 | Core Framework | C++ Character/Controller + Enhanced Input | ✅ Done |
| Day 2 | GAS + Spectral Shaders | GAS Foundation + Arcane Toggle + Post-Process | ✅ Done |
| Day 3 | Grid + Camera | `ATaeGridCube` + `ATaeGridManager` + over-shoulder camera | ✅ Done |
| Day 4 | Data-Driven UI | MVVM Viewmodel + Common UI HUD + Grid DataTable | ✅ Done |
| Day 5 | Portal & Polish | Render-to-Texture Portal + Win Condition UI | ✅ Done |

### Sprint 2 — Enhancement
*Optional. Adds depth and polish once the core slice is complete.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 6 | Animation (Motion Matching) | Motion Matching locomotion + Arcane float + footsteps | 🔄 In progress |
| Day 7 | Level Dressing & Clutter | Hand-placed island set dressing + `ATaeClutterScatter` random props | ⬜ Not started |
| Day 8 | Interactables & Connections | Chest/pickup spawners + `ATaeWorldManager` hidden root-paths | ⬜ Not started |
| Day 9 | Audio & SFX | UI sounds + portal/spectral SFX + ambient music | ⬜ Not started |

---

## Day 1 — Core Framework ✅

**Goal:** Playable character in-editor with all input wired end-to-end.

**All complete.**

- C++ framework classes — `ATaeCharacter`, `ATaePlayerController`, `ATaeGameMode` (`AGameMode`), `ATaeGameState`, `UTaeGameInstance`, `ATaeHud`
- Enhanced Input — Move / Look / Jump / `DoSpectralShift` stub bound in controller; WASD Swizzle/Negate modifiers correct
- `LogTae` project log category; null-guard warnings on all BP-assigned properties → migrated to `IsDataValid` on `ATaePlayerController`
- Blueprint parenting — `BP_TaeGameMode`, `BP_Hero`, `BP_TaePlayerController`, `BP_TaeGameState`, `BP_TaeGameInstance`, `BP_TaeHud` created with correct parents; class refs set in `BP_TaeGameMode` Class Defaults
- Input assets — `IMC_Default`, `IA_MoveInputAction`, `IA_LookInputAction`, `IA_JumpInputAction`, `IA_SpectralShift`
- Config — `GlobalDefaultGameMode`, Enhanced Input component class, `GameInstanceClass` set
- `UTaeMainMenuWidget` (`UCommonActivatableWidget`) + `WBP_MainMenu` placeholder — Start (DeactivateWidget) / Exit (QuitGame); shown via `ATaeHud::BeginPlay`; `CommonUI` added to `Build.cs`

---

## Day 2 — GAS + Spectral Shaders

**Goal:** GAS drives the Arcane toggle; pressing a key activates `GA_SpectralShift`, grants `Arcane.Vision` tag, and triggers the post-process pass.

**Before starting — loose ends from Day 1:** ✅ All clear

**All complete.**

### C++ / GAS
- GAS deps, ASC + `UTaeManaAttributeSet` + `UGA_SpectralShift` + `UTaeStateComponent`
- `UTaeGameplayAbility` abstract base — `AbilityDuration` timer (`0` = player-cancel only); all future abilities inherit from this
- `UGA_SpectralShift` → `UTaeGameplayAbility`; toggle via `TAG_Arcane_Vision`; `IMC_Arcane` push/pop in Activate/End
- Native tag `TAG_Arcane_Vision` declared in `TaeGASTypes.h`, defined in `TaeGASTypes.cpp` — no `FName` strings in calling code
- `ATaeCharacter` exposes `SpectralShiftHandle`; controller uses `TryActivateAbility` / `CancelAbilityHandle` — no class-based lookup
- `SpectralShiftAbility` (`TSubclassOf`) on `ATaeCharacter` — assign `BP_GA_SpectralShift` in `BP_Hero` Class Defaults

### Materials / Rendering
- `M_SpectralEdge` — animated plasma overlay; DDX/DDY depth edge mask; `Floor(Time)` stepped animation
- `BP_SpectralVolume` — Infinite Extent post-process volume; chromatic aberration; disabled by default; toggled by `BP_GA_SpectralShift`

---

## Day 3 — Grid + Camera ✅

**Goal:** Procedural cube grid loads and reacts to Arcane Vision. Close over-shoulder third-person camera. Devlog gif ready.

**Complete** — mesh collision deferred; placeholder meshes are complex shapes, add box/convex collision per mesh when final assets are ready.

### C++
- `ATaeGridCube` — Actor with `UStaticMeshComponent` + `UTaeStateComponent`; responds to `OnStateChanged`; toggles `ECollisionEnabled::NoCollision` when hidden, restores on Arcane off
- `ATaeGridManager` — spawns an N×M×K grid of `ATaeGridCube` from configurable `UPROPERTY` dimensions; no DataTable yet
- `ATaeCharacter` — `USpringArmComponent` + `UCameraComponent` close over-shoulder setup (replaces first-person camera)

### Materials
> Two separate materials swapped by `ATaeGridCube` via `UStaticMeshComponent::SetMaterial()` on `OnStateChanged`.
- `M_GridCube_Forest` — default world state; Surface opaque, organic greens and rock tones
- `M_GridCube_Arcane` — Arcane vision state; Surface translucent with emissive glow

### Editor
- `BP_GridCube` → parent `ATaeGridCube`; test cube mesh + materials assigned (rework planned with detailed materials)
- `BP_GridManager` — placed in level
- `BP_Hero` — tree skeletal mesh + animation assigned

---

## Day 4 — Data-Driven UI ✅

**Goal:** HUD reacts to mana/vision state with zero Tick usage. Grid layout driven by DataTable.

**All complete.**

### C++ — MVVM
- `ModelViewViewModel` added to `Build.cs`; `UTaeStateComponent` migrated to `TAG_Arcane_Vision` native tag
- `UTaeHudViewModel` — `UMVVMViewModelBase`; `Mana` (float) + `bArcaneActive` (bool) with `FieldNotify`; setters use `UE_MVVM_SET_PROPERTY_VALUE`
- `UTaeGameInstance` — creates and owns `UTaeHudViewModel` in `Init()`; `GetHudViewModel()` exposed as `BlueprintCallable`
- `ATaePlayerController::SetPawn` — wires ASC tag + Mana attribute delegates → ViewModel via `AddWeakLambda`; pushes initial values on possession
- `ATaeHud` — `HudWidgetClass` (`TSubclassOf<UUserWidget>`); creates and adds `WBP_HUD` to player screen in `BeginPlay`

### Editor / UMG
- `WBP_HUD` — View Bindings: `ManaText` → `TextBlock.Text`, `ArcaneVisibility` → widget visibility; Event Construct sets ViewModel from GameInstance
- `BP_TaeHud` — `WBP_HUD` assigned to `HudWidgetClass`
- `WBP_PauseMenu` — `UTaeActivatableWidget`; Escape via `IA_Pause` → `ATaeHud::TogglePauseMenu`
- `WBP_VictoryScreen` — `UTaeActivatableWidget`; "To Main Screen" → `ATaeHud::ShowMainMenu`
- `UTaeActivatableWidget` — base class; auto collapse/visible on deactivate/activate; `WBP_MainMenu`, `WBP_PauseMenu`, `WBP_VictoryScreen` all inherit


---

## Day 5 — Portal & Polish ✅

**Goal:** Functional end-portal with render-to-texture view; win condition triggers Victory screen.

**Complete** — render-to-texture portal, `M_Portal`, and the `M_GridCube_Forest` texture upgrade deferred (not needed for win condition).

### C++
- `ATaePortal` — Actor with `USphereComponent` trigger; overlap calls `ATaeHud::ShowVictoryScreen()`
- `UTaeArcaneSubsystem` (`UWorldSubsystem`) — auto-finds `APostProcessVolume` in `OnWorldBeginPlay`; `SetArcaneActive` toggles volume + crossfades music; `FlashVignette` spikes vignette intensity then fades
- `UTaeGameInstance` — `Music_Forest`, `Music_Arcane` (EditDefaultsOnly), `MusicCrossfadeDuration`; `GA_SpectralShift` calls subsystem instead of holding direct PP reference

### Deferred
- [ ] `ATaePortal` render-to-texture — `USceneCaptureComponent2D` + `UTextureRenderTarget2D`
- [ ] `M_Portal` — samples `UTextureRenderTarget2D`; distortion/chromatic aberration pass
- [ ] `M_GridCube_Forest` texture upgrade — replace Voronoi with tiling rust + moss textures + normal maps

### Audio
> Asset import/wiring moved to Day 9 — see below. Music crossfade C++ logic is complete.
- Music crossfade logic — `UAudioComponent` pair in `UTaeArcaneSubsystem`; spawned in `OnWorldBeginPlay`; crossfaded via `SetArcaneActive`

### Substrate Upgrade
- `M_GridCube_Forest` → Substrate opaque slab
- `M_GridCube_Arcane` → Substrate translucent slab with emissive layer
- `M_SpectralEdge` — verified and converted to Substrate

### Polish
- Screen-space vignette flash when Spectral Shift activates — `UTaeArcaneSubsystem::FlashVignette()`
- `DefaultGame.ini` `ProjectVersion` bump to `0.1.0`

---

## Day 6 — Animation (Motion Matching)

**Goal:** Ant's movement feels heavy and rooted in normal mode, reaching and extending in Arcane mode. Motion Matching drives both states from separate pose databases.

> Depends on Day 2 (`GameplayTag.Arcane.Vision` used to blend between databases).

### Setup
- [ ] Enable `PoseSearch` plugin in `ThroughArcaneEyes.uproject`
- [ ] Add `PoseSearch` to `Build.cs` public deps

### Pose Databases
- [ ] `PSD_Locomotion` — normal movement; heavy root-gait walk, idle sway, grounded feel
- [ ] `PSD_ArcaneReach` — Arcane mode; root-reach/extend poses, slow deliberate movement, no hard foot plant

### C++
- [ ] `ATaeCharacter` — expose `bArcaneActive` for the Animation Blueprint (via ASC tag query)
- [ ] Tune `UCharacterMovementComponent` for Arcane mode — reduced gravity scale, increased air control, lower max walk speed; applied/removed by `UGA_SpectralShift`

### Editor
- [ ] `ABP_Hero` Animation Blueprint — Motion Matching node selecting between `PSD_Locomotion` and `PSD_ArcaneReach` based on `bArcaneActive`
- [ ] Blend time between databases (suggested: 0.3–0.5s) to avoid snapping on toggle
- [ ] Assign `ABP_Hero` to `BP_Hero` skeletal mesh
- [ ] `S_Footstep_Root` — heavy root footstep set (4–6 variations); played via `AnimNotify_PlaySound` in `ABP_Hero`

---

## Day 7 — Level Dressing & Clutter

**Goal:** Replace procedural grid-cube islands with hand-placed rock/ruin set dressing, plus a randomised small-prop scatter system for clutter. Full proposal in [docs/LevelGeneration.md](LevelGeneration.md).

> Retires `ATaeGridManager`, `FTaeIslandArchetype`, `DT_IslandArchetypes`, and the `DecayFactor` roll from the earlier procedural plan. `ATaeGridCube`'s reveal pattern survives — reused by `ATaeRootPath` in Day 8.

- [ ] Hand-place island set dressing — `BP_Rock_*`, `BP_Ruin_*` static meshes arranged manually per island in the level (no spawner code)
- [ ] `ATaeClutterScatter` — Actor with bounds (box/spline) + mesh pool (small rocks, wire piles, scrap); scatters N instances via `UInstancedStaticMeshComponent` on construction; per-instance `FRandomStream` seed for reproducible placement; optional ground-snap via line trace
- [ ] `BP_ClutterScatter_Rocks`, `BP_ClutterScatter_Wires` — placed per island, density/seed tuned per instance

## Day 8 — Interactables & Connections

**Goal:** Chests and pickups spawn semi-randomly at designer-placed markers. Top-level manager tracks the hidden root-paths between islands, revealed by Arcane Vision (mechanic unchanged from the original proposal).

- [ ] `ATaeInteractableSpawnPoint` — lightweight marker actor; designer places more per island than the number that will actually spawn
- [ ] `ATaeInteractableSpawner` — collects all spawn points at `BeginPlay`; rolls `SpawnChance` per point (`FRandomStream`); picks a random interactable class from a weighted pool (`FTaeInteractableEntry { Class, Weight }`) for each point that hits
- [ ] `BP_Chest`, `BP_Pickup_Mana` — interactable Blueprints
- [ ] `ATaeWorldManager` — one per level; holds references to `ATaeRootPath` actors (hidden root connections between islands)
- [ ] `ATaeRootPath` — spline-based actor representing one hidden root connection; placed by a designer; hidden by default (visibility + collision off), revealed via `UTaeStateComponent` listening for `Arcane.Vision` (same reveal pattern as `ATaeGridCube`)

---

## Day 9 — Audio & SFX

**Goal:** Full audio pass — UI clicks, spectral shift whooshes, portal ambience, grid reveal chime, ambient music, victory sting.

> Import assets as `.wav`; wire via `UGameplayStatics::PlaySound2D` or `UAudioComponent`. No custom audio C++ needed.
> Music crossfade C++ (`UTaeArcaneSubsystem`) is already wired — only asset assignment remains.
> Candidate source files for each cue: see [docs/AudioAssets.md](AudioAssets.md).

### UI Sounds
- [ ] `S_UI_Click` — button hover/confirm SFX; bound to widget events in `WBP_MainMenu` / `WBP_PauseMenu`

### Spectral SFX
- [ ] `S_SpectralShift_On` + `S_SpectralShift_Off` — magical whoosh on toggle; played in `UGA_SpectralShift`
- [ ] `S_GridReveal` — crystalline chime when hidden cubes materialise; played in `BP_GridCube` on state change

### Portal & Win
- [ ] `S_Portal_Ambience` — dimensional hum loop; `UAudioComponent` on `BP_Portal`
- [ ] `S_Victory` — short magical flourish triggered on win condition

### Music
- [ ] `Music_Forest` — mysterious ambient loop (normal mode); assign in `BP_TaeGameInstance`
- [ ] `Music_Arcane` — ethereal loop (Arcane mode); assign in `BP_TaeGameInstance`; crossfades via `UTaeArcaneSubsystem::SetArcaneActive`
