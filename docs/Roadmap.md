# Development Roadmap — Through Arcane Eyes

> For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

### Sprint 1 — Core Vertical Slice
*Playable portfolio demo: character, Arcane mechanic, grid world, UI, portal.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 1 | Core Framework | C++ Character/Controller + Enhanced Input | ✅ Done |
| Day 2 | GAS + Spectral Shaders | GAS Foundation + Arcane Toggle + Post-Process | ⬜ Not started |
| Day 3 | Grid & Interaction | `AGridCube` + hidden/solid state logic | ⬜ Not started |
| Day 4 | Data-Driven UI | MVVM Viewmodel + Common UI HUD | ⬜ Not started |
| Day 5 | Portal & Polish | Render-to-Texture Portal + Win Condition UI | ⬜ Not started |

### Sprint 2 — Enhancement
*Optional. Adds depth and polish once the core slice is complete.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 6 | Animation | Motion Matching — realistic locomotion + Arcane float | ⬜ Not started |

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

### GAS Foundation (C++)
- [x] Add `GameplayAbilities`, `GameplayTags`, `GameplayTasks` to `Build.cs` public deps
- [x] `UAbilitySystemComponent` added to `ATaeCharacter`; implement `IAbilitySystemInterface`
- [x] `UTaeManaAttributeSet` — `Mana` + `MaxMana` attributes, clamped; shared `ATTRIBUTE_ACCESSORS` macro in `TaeGASTypes.h`; feeds Day 4 ViewModel directly
- [x] `UGA_SpectralShift` — grants/removes `Arcane.Vision` tag, pushes/pops `IMC_Arcane`; `InstancedPerActor`
- [x] `Arcane.Vision` tag registered in `Config/DefaultGameplayTags.ini`
- [x] `DoSpectralShift` in `ATaePlayerController` — toggles via tag check + `FindAbilitySpecFromClass` / `CancelAbility`

### State Propagation (C++)
- [x] `UTaeStateComponent` — `UActorComponent`; registers `RegisterGameplayTagEvent(Arcane.Vision)`, broadcasts `OnArcaneStateChanged`

### Materials / Rendering
- [ ] Post-Process Material `M_SpectralEdge` — Sobel-edge detection algorithm
- [x] Custom Depth Stencil — `r.CustomDepth=3` set in `DefaultEngine.ini`; convention: `1` = hidden geometry, `2` = portal
- [ ] `BP_SpectralVolume` — Post-Process Volume enabled/disabled by `UGA_SpectralShift`

---

## Day 3 — Grid & Interaction

**Goal:** Procedural cube grid loads; cubes toggle hidden/solid via the Spectral state.

### C++
- [ ] `ATaeGridCube` — Actor with `UStaticMeshComponent` + `UTaeStateComponent`; responds to `OnStateChanged`
- [ ] `ATaeGridManager` — spawns an N×N×N grid of `ATaeGridCube` from a `DataTable` or procedural config
- [ ] Collision toggle — `ATaeGridCube` disables `ECollisionEnabled::NoCollision` when hidden, restores on Arcane off

### Materials (Substrate)
> Substrate is already enabled in `DefaultEngine.ini`. Two separate materials swapped by `ATaeGridCube` via `UStaticMeshComponent::SetMaterial()` on `OnStateChanged`.
- [ ] `M_GridCube_Forest` — default world state; Substrate opaque slab, organic greens and rock tones
- [ ] `M_GridCube_Arcane` — Arcane vision state; Substrate translucent slab with emissive layer for glow effect

### Editor
- [ ] `DT_GridLayout` DataTable (row struct `FTaeGridRow`) — defines which cubes start hidden
- [ ] `BP_GridCube` → parent `ATaeGridCube`; assign mesh + Substrate materials
- [ ] `BP_GridManager` placed in level

---

## Day 4 — Data-Driven UI

**Goal:** HUD reacts to mana/vision state with zero Tick usage.

### C++
- [ ] `UTaeHudViewModel` — extends `UMVVMViewModelBase`; exposes `Mana` (float) and `bArcaneActive` (bool) with `FIELD_NOTIFY`
- [ ] `UTaeGameInstance` — owns and initialises `UTaeHudViewModel`
- [ ] Connect `UTaeStateComponent::OnStateChanged` → ViewModel setter

### Editor / UMG
- [ ] `WBP_HUD` — main UMG widget; binds to `UTaeHudViewModel` via MVVM binding panel (no `Tick`)
- [ ] `WBP_PauseMenu` — Common UI `UCommonActivatableWidget`; handles input focus automatically
- [ ] `WBP_VictoryScreen` — Common UI activatable; triggered by win condition

### Config
- [ ] Add `CommonUI` and `ModelViewViewModel` to `ThroughArcaneEyes.Build.cs` public deps

---

## Day 5 — Portal & Polish

**Goal:** Functional end-portal with render-to-texture view; win condition triggers Victory screen.

### C++
- [ ] `ATaePortal` — Actor with `USceneCaptureComponent2D` rendering to a `UTextureRenderTarget2D`
- [ ] Win condition — overlap trigger calls `UTaeGameInstance` → activates `WBP_VictoryScreen`

### Materials
- [ ] `M_Portal` — samples `UTextureRenderTarget2D`; distortion/chromatic aberration pass

### Polish
- [ ] Ambient sound attenuation on grid cubes
- [ ] Screen-space feedback when Spectral Shift activates (vignette flash)
- [ ] `DefaultGame.ini` `ProjectVersion` bump to `0.1.0`

---

## Day 6 — Animation

**Goal:** Character movement feels grounded in normal mode and weightless/magical in Arcane mode, driven by Motion Matching.

> Depends on Day 2 (GAS tag `GameplayTag.Arcane.Vision` used to blend between databases).

### Setup
- [ ] Enable `PoseSearch` plugin in `ThroughArcaneEyes.uproject`
- [ ] Add `PoseSearch` to `Build.cs` public deps

### Pose Databases
- [ ] `PSD_Locomotion` — normal movement; idle, walk, run, jump cycles; realistic forest traversal feel
- [ ] `PSD_ArcaneFloat` — Arcane mode; slow drift, hover idle, gliding movement; no hard foot plant

### C++
- [ ] `ATaeCharacter` — expose `bArcaneActive` state for the Animation Blueprint to read (via ASC tag query or a replicated bool)
- [ ] Tune `UCharacterMovementComponent` for Arcane mode — reduced gravity scale, increased air control, lower max walk speed; applied/removed by `UGA_ArcaneShift`

### Editor
- [ ] `ABP_Hero` Animation Blueprint — Motion Matching node selecting between `PSD_Locomotion` and `PSD_ArcaneFloat` based on `bArcaneActive`
- [ ] Blend time between databases (suggested: 0.3–0.5s) to avoid snapping on toggle
- [ ] Assign `ABP_Hero` to `BP_Hero` skeletal mesh
