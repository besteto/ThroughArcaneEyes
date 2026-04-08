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
| Day 5 | Portal & Polish | Render-to-Texture Portal + Win Condition UI | 🔄 In progress |

### Sprint 2 — Enhancement
*Optional. Adds depth and polish once the core slice is complete.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 6 | Animation (Motion Matching) | Motion Matching locomotion + Arcane float + footsteps | ⬜ Not started |
| Day 7 | Hierarchical World System | `ATaeIsland` + `FTaeIslandArchetype` + DataTable-driven spawning | ⬜ Not started |
| Day 8 | World Manager & Connections | `ATaeWorldManager` + hidden root-paths between islands | ⬜ Not started |

---

## Day 1 — Core Framework ✅

**Goal:** Playable character in-editor with all input wired end-to-end.

- [x] C++ framework classes — `ATaeCharacter`, `ATaePlayerController`, `ATaeGameMode` (`AGameMode`), `ATaeGameState`, `UTaeGameInstance`, `ATaeHud`
- [x] Enhanced Input — Move / Look / Jump / `DoSpectralShift` stub bound in controller; WASD Swizzle/Negate modifiers correct
- [x] `LogTae` project log category; null-guard warnings on all BP-assigned properties → migrated to `IsDataValid` on `ATaePlayerController`
- [x] Blueprint parenting — `BP_TaeGameMode`, `BP_Hero`, `BP_TaePlayerController`, `BP_TaeGameState`, `BP_TaeGameInstance`, `BP_TaeHud` created with correct parents; class refs set in `BP_TaeGameMode` Class Defaults
- [x] Input assets — `IMC_Default`, `IA_MoveInputAction`, `IA_LookInputAction`, `IA_JumpInputAction`, `IA_SpectralShift`
- [x] Config — `GlobalDefaultGameMode`, Enhanced Input component class, `GameInstanceClass` set
- [x] `UTaeMainMenuWidget` (`UCommonActivatableWidget`) + `WBP_MainMenu` placeholder — Start (DeactivateWidget) / Exit (QuitGame); shown via `ATaeHud::BeginPlay`; `CommonUI` added to `Build.cs`

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

### C++
- [x] `ATaeGridCube` — Actor with `UStaticMeshComponent` + `UTaeStateComponent`; responds to `OnStateChanged`
- [x] `ATaeGridManager` — spawns an N×M×K grid of `ATaeGridCube` from configurable `UPROPERTY` dimensions; no DataTable yet
- [x] Collision toggle — `ATaeGridCube` sets `ECollisionEnabled::NoCollision` when hidden, restores on Arcane off
- [ ] Mesh collision — deferred; placeholder meshes are complex shapes; add box/convex collision per mesh when final assets are ready
- [x] `ATaeCharacter` — `USpringArmComponent` + `UCameraComponent` close over-shoulder setup (replaces first-person camera)

### Materials
> Two separate materials swapped by `ATaeGridCube` via `UStaticMeshComponent::SetMaterial()` on `OnStateChanged`.
- [x] `M_GridCube_Forest` — default world state; Surface opaque, organic greens and rock tones
- [x] `M_GridCube_Arcane` — Arcane vision state; Surface translucent with emissive glow

### Editor
- [x] `BP_GridCube` → parent `ATaeGridCube`; test cube mesh + materials assigned (rework planned with detailed materials)
- [x] `BP_GridManager` — placed in level
- [x] `BP_Hero` — tree skeletal mesh + animation assigned

---

## Day 4 — Data-Driven UI

**Goal:** HUD reacts to mana/vision state with zero Tick usage. Grid layout driven by DataTable.

### C++ — MVVM (complete)
- [x] `ModelViewViewModel` added to `Build.cs`; `UTaeStateComponent` migrated to `TAG_Arcane_Vision` native tag
- [x] `UTaeHudViewModel` — `UMVVMViewModelBase`; `Mana` (float) + `bArcaneActive` (bool) with `FieldNotify`; setters use `UE_MVVM_SET_PROPERTY_VALUE`
- [x] `UTaeGameInstance` — creates and owns `UTaeHudViewModel` in `Init()`; `GetHudViewModel()` exposed as `BlueprintCallable`
- [x] `ATaePlayerController::SetPawn` — wires ASC tag + Mana attribute delegates → ViewModel via `AddWeakLambda`; pushes initial values on possession
- [x] `ATaeHud` — `HudWidgetClass` (`TSubclassOf<UUserWidget>`); creates and adds `WBP_HUD` to player screen in `BeginPlay`

### Editor / UMG
- [x] `WBP_HUD` — View Bindings: `ManaText` → `TextBlock.Text`, `ArcaneVisibility` → widget visibility; Event Construct sets ViewModel from GameInstance
- [x] `BP_TaeHud` — `WBP_HUD` assigned to `HudWidgetClass`
- [x] `WBP_PauseMenu` — `UTaeActivatableWidget`; Escape via `IA_Pause` → `ATaeHud::TogglePauseMenu`
- [x] `WBP_VictoryScreen` — `UTaeActivatableWidget`; "To Main Screen" → `ATaeHud::ShowMainMenu`
- [x] `UTaeActivatableWidget` — base class; auto collapse/visible on deactivate/activate; `WBP_MainMenu`, `WBP_PauseMenu`, `WBP_VictoryScreen` all inherit


---

## Day 5 — Portal & Polish

**Goal:** Functional end-portal with render-to-texture view; win condition triggers Victory screen.

### C++
- [x] `ATaePortal` — Actor with `USphereComponent` trigger; overlap calls `ATaeHud::ShowVictoryScreen()`
- [x] `UTaeArcaneSubsystem` (`UWorldSubsystem`) — auto-finds `APostProcessVolume` in `OnWorldBeginPlay`; `SetArcaneActive` toggles volume + crossfades music; `FlashVignette` spikes vignette intensity then fades
- [x] `UTaeGameInstance` — `Music_Forest`, `Music_Arcane` (EditDefaultsOnly), `MusicCrossfadeDuration`; `GA_SpectralShift` calls subsystem instead of holding direct PP reference
- [ ] `ATaePortal` render-to-texture — `USceneCaptureComponent2D` + `UTextureRenderTarget2D` (deferred, not needed for win condition)

### Materials
- [ ] `M_Portal` — samples `UTextureRenderTarget2D`; distortion/chromatic aberration pass

### Audio
> Import assets as `.wav`; wire via `UGameplayStatics::PlaySound2D` or `UAudioComponent`. No custom audio C++ needed this day.
- [ ] `S_UI_Click` — button hover/confirm SFX; bound to widget events in `WBP_MainMenu` / `WBP_PauseMenu`
- [ ] `S_SpectralShift_On` + `S_SpectralShift_Off` — magical whoosh on toggle; played in `UGA_SpectralShift`
- [ ] `S_GridReveal` — crystalline chime when hidden cubes materialise; played in `BP_GridCube` on state change
- [ ] `Music_Forest` — mysterious ambient loop (normal mode)
- [ ] `Music_Arcane` — ethereal loop (Arcane mode); crossfades with `Music_Forest` on `Arcane.Vision` tag change
- [ ] `S_Portal_Ambience` — dimensional hum loop; `UAudioComponent` on `BP_Portal`
- [ ] `S_Victory` — short magical flourish triggered on win condition
- [x] Music crossfade logic — `UAudioComponent` pair in `UTaeArcaneSubsystem`; spawned in `OnWorldBeginPlay`; crossfaded via `SetArcaneActive`

### Substrate Upgrade
- [x] `M_GridCube_Forest` → Substrate opaque slab
- [x] `M_GridCube_Arcane` → Substrate translucent slab with emissive layer
- [x] `M_SpectralEdge` — verified and converted to Substrate

### Polish
- [x] Screen-space vignette flash when Spectral Shift activates — `UTaeArcaneSubsystem::FlashVignette()`
- [ ] `DefaultGame.ini` `ProjectVersion` bump to `0.1.0`
- [ ] `M_GridCube_Forest` texture upgrade — replace Voronoi with tiling rust + moss textures + normal maps

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

## Day 7 — Hierarchical World System

**Goal:** Replace manual island placement with a data-driven, modular system. Full proposal in `docs/HierarchicalIsland.txt`.

- [ ] `ATaeIsland` — replaces `ATaeGridManager`; owns `UTaeStateComponent`; spawns cube grid from archetype; broadcasts state to child cubes directly (cubes no longer register with GAS themselves)
- [ ] `FTaeIslandArchetype` — `USTRUCT` DataTable row; fields: `GridDimensions`, `DecayFactor` (float 0–1), `bIsPuzzleIsland`
- [ ] `DT_IslandArchetypes` — DataTable of island archetypes (e.g. Rusty Factory, Mossy Overgrowth)
- [ ] `DecayFactor` spawning — roll against `FRandomStream` seed; spawn `BP_GridCube` or skip/replace with junk mesh
- [ ] Per-archetype material override — Island passes archetype materials to each spawned GridCube at spawn time

## Day 8 — World Manager & Connections

**Goal:** Top-level manager tracks all islands and the hidden root-paths between them.

- [ ] `ATaeWorldManager` — one per level; holds references to all `ATaeIsland` actors + hidden connection path data
- [ ] Connection paths hidden by default; revealed (visibility + collision) when `Arcane.Vision` tag is active
- [ ] `BP_IndustrialJunk` — Nanite-enabled junk mesh actor; spawned by Island when `DecayFactor` roll fails
