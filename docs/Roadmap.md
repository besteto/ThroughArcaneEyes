# Development Roadmap — Through Arcane Eyes

> For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

### Sprint 1 — Core Vertical Slice
*Playable portfolio demo: character, Arcane mechanic, grid world, UI, portal.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 1 | Core Framework | C++ Character/Controller + Enhanced Input | ✅ Done |
| Day 2 | GAS + Spectral Shaders | GAS Foundation + Arcane Toggle + Post-Process | ✅ Done |
| Day 3 | Grid + Camera | `ATaeGridCube` + `ATaeGridManager` + over-shoulder camera | 🔄 In Progress |
| Day 4 | Data-Driven UI | MVVM Viewmodel + Common UI HUD + Grid DataTable | ⬜ Not started |
| Day 5 | Portal & Polish | Render-to-Texture Portal + Win Condition UI | ⬜ Not started |

### Sprint 2 — Enhancement
*Optional. Adds depth and polish once the core slice is complete.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 6 | Animation (Motion Matching) | Motion Matching locomotion + Arcane float + footsteps | ⬜ Not started |

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
- `BP_SpectralVolume` — Infinite Extent post-process volume; disabled by default; toggled by `BP_GA_SpectralShift`

---

## Day 3 — Grid + First-Person Hands

**Goal:** Procedural cube grid loads and reacts to Arcane Vision. Close over-shoulder third-person camera. Devlog gif ready.

### C++
- [x] `ATaeGridCube` — Actor with `UStaticMeshComponent` + `UTaeStateComponent`; responds to `OnStateChanged`
- [x] `ATaeGridManager` — spawns an N×M×K grid of `ATaeGridCube` from configurable `UPROPERTY` dimensions; no DataTable yet
- [x] Collision toggle — `ATaeGridCube` sets `ECollisionEnabled::NoCollision` when hidden, restores on Arcane off
- [x] `ATaeCharacter` — `USpringArmComponent` + `UCameraComponent` close over-shoulder setup (replaces first-person camera)

### Materials (Substrate)
> Substrate is already enabled in `DefaultEngine.ini`. Two separate materials swapped by `ATaeGridCube` via `UStaticMeshComponent::SetMaterial()` on `OnStateChanged`.
- [ ] `M_GridCube_Forest` — default world state; Substrate opaque slab, organic greens and rock tones
- [ ] `M_GridCube_Arcane` — Arcane vision state; Substrate translucent slab with emissive layer for glow effect

### Editor
- [ ] `BP_GridCube` → parent `ATaeGridCube`; assign mesh + Substrate materials
- [ ] `BP_GridManager` — place in level; set grid dimensions
- [ ] `BP_Hero` — verify spring arm length + socket offset in viewport

---

## Day 4 — Data-Driven UI

**Goal:** HUD reacts to mana/vision state with zero Tick usage. Grid layout driven by DataTable.

### C++ — Grid config
- [ ] `FTaeGridRow` struct — `USTRUCT`; fields: `GridPosition (FIntVector)`, `bStartHidden (bool)`
- [ ] `DT_GridLayout` DataTable using `FTaeGridRow` — defines which cubes start hidden
- [ ] `ATaeGridManager` — wire to read from `DT_GridLayout`; fall back to procedural fill if table is empty

### C++ — MVVM
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

### Audio
> Import assets as `.wav`; wire via `UGameplayStatics::PlaySound2D` or `UAudioComponent`. No custom audio C++ needed this day.
- [ ] `S_UI_Click` — button hover/confirm SFX; bound to widget events in `WBP_MainMenu` / `WBP_PauseMenu`
- [ ] `S_SpectralShift_On` + `S_SpectralShift_Off` — magical whoosh on toggle; played in `UGA_SpectralShift`
- [ ] `S_GridReveal` — crystalline chime when hidden cubes materialise; played in `BP_GridCube` on state change
- [ ] `Music_Forest` — mysterious ambient loop (normal mode)
- [ ] `Music_Arcane` — ethereal loop (Arcane mode); crossfades with `Music_Forest` on `Arcane.Vision` tag change
- [ ] `S_Portal_Ambience` — dimensional hum loop; `UAudioComponent` on `BP_Portal`
- [ ] `S_Victory` — short magical flourish triggered on win condition
- [ ] Music crossfade logic — `UAudioComponent` pair on `BP_TaeHud`; faded via `UTaeStateComponent::OnArcaneStateChanged`

### Polish
- [ ] Screen-space vignette flash when Spectral Shift activates
- [ ] `DefaultGame.ini` `ProjectVersion` bump to `0.1.0`

---

## Day 6 — Animation (Motion Matching)

**Goal:** Character movement feels grounded in normal mode and weightless/magical in Arcane mode, driven by Motion Matching. Robed figure benefits from cloth sim + drift.

> Depends on Day 2 (`GameplayTag.Arcane.Vision` used to blend between databases).

### Setup
- [ ] Enable `PoseSearch` plugin in `ThroughArcaneEyes.uproject`
- [ ] Add `PoseSearch` to `Build.cs` public deps

### Pose Databases
- [ ] `PSD_Locomotion` — normal movement; idle, walk, run, jump; grounded forest traversal feel
- [ ] `PSD_ArcaneFloat` — Arcane mode; slow drift, hover idle, gliding; no hard foot plant

### C++
- [ ] `ATaeCharacter` — expose `bArcaneActive` for the Animation Blueprint (via ASC tag query)
- [ ] Tune `UCharacterMovementComponent` for Arcane mode — reduced gravity scale, increased air control, lower max walk speed; applied/removed by `UGA_SpectralShift`

### Editor
- [ ] `ABP_Hero` Animation Blueprint — Motion Matching node selecting between `PSD_Locomotion` and `PSD_ArcaneFloat` based on `bArcaneActive`
- [ ] Blend time between databases (suggested: 0.3–0.5s) to avoid snapping on toggle
- [ ] Assign `ABP_Hero` to `BP_Hero` skeletal mesh
- [ ] `S_Footstep_Robe` — soft cloth footstep set (4–6 variations); played via `AnimNotify_PlaySound` in `ABP_Hero`
