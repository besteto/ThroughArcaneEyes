# Development Roadmap — Through Arcane Eyes

> For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

### Sprint 1 — Core Vertical Slice
*Playable portfolio demo: character, Arcane mechanic, grid world, UI, portal.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 1 | Core Framework | C++ Character/Controller + Enhanced Input | ✅ Done |
| Day 2 | GAS + Spectral Shaders | GAS Foundation + Arcane Toggle + Post-Process | ✅ Done |
| Day 3 | Grid + First-Person Hands | `ATaeGridCube` + `ATaeGridManager` + Arms mesh wired | 🔄 In Progress |
| Day 4 | Data-Driven UI | MVVM Viewmodel + Common UI HUD + Grid DataTable | ⬜ Not started |
| Day 5 | Portal & Polish | Render-to-Texture Portal + Win Condition UI | ⬜ Not started |

### Sprint 2 — Enhancement
*Optional. Adds depth and polish once the core slice is complete.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 6 | Arms Animation + Camera Feel | Arms ABP + procedural camera bob + Arcane float tuning | ⬜ Not started |

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

**C++ complete** — GAS deps, ASC + `UTaeManaAttributeSet` + `UGA_SpectralShift` + `UTaeStateComponent` all done; `Arcane.Vision` tag confirmed working in PIE.

### Materials / Rendering (remaining)
- [ ] Post-Process Material `M_SpectralEdge` — Sobel-edge detection on Custom Depth (`r.CustomDepth=3` already set)
- [ ] `BP_SpectralVolume` — Post-Process Volume; `BP_GA_SpectralShift` enables/disables it; assign `IMC_Arcane`

---

## Day 3 — Grid + First-Person Hands

**Goal:** Procedural cube grid loads and reacts to Arcane Vision. Player sees their own hands. Devlog gif ready.

### C++
- [x] `ATaeGridCube` — Actor with `UStaticMeshComponent` + `UTaeStateComponent`; responds to `OnStateChanged`
- [x] `ATaeGridManager` — spawns an N×M×K grid of `ATaeGridCube` from configurable `UPROPERTY` dimensions; no DataTable yet
- [x] Collision toggle — `ATaeGridCube` sets `ECollisionEnabled::NoCollision` when hidden, restores on Arcane off
- [x] `ATaeCharacter` — add `ArmsMesh USkeletalMeshComponent`; attach to camera socket; `bOnlyOwnerSee = true`; main mesh `bOwnerNoSee = true`

### Materials (Substrate)
> Substrate is already enabled in `DefaultEngine.ini`. Two separate materials swapped by `ATaeGridCube` via `UStaticMeshComponent::SetMaterial()` on `OnStateChanged`.
- [ ] `M_GridCube_Forest` — default world state; Substrate opaque slab, organic greens and rock tones
- [ ] `M_GridCube_Arcane` — Arcane vision state; Substrate translucent slab with emissive layer for glow effect

### Editor
- [ ] `BP_GridCube` → parent `ATaeGridCube`; assign mesh + Substrate materials
- [ ] `BP_GridManager` — place in level; set grid dimensions
- [ ] `BP_Hero` — assign hands skeletal mesh to `ArmsMesh`; position against camera

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

### Polish
- [ ] Ambient sound attenuation on grid cubes
- [ ] Screen-space feedback when Spectral Shift activates (vignette flash)
- [ ] `DefaultGame.ini` `ProjectVersion` bump to `0.1.0`

---

## Day 6 — Arms Animation + Camera Feel

**Goal:** Hands animate naturally; camera movement feels weighted in normal mode and drifting in Arcane mode. No full-body mesh or Motion Matching needed.

> Depends on Day 3 (`ArmsMesh` wired to camera) and Day 2 (`GameplayTag.Arcane.Vision` driving state).

### C++
- [ ] `ATaeCharacter` — tune `UCharacterMovementComponent` for Arcane mode: reduced gravity scale, increased air control, lower max walk speed; applied/removed by `UGA_SpectralShift` on tag grant/remove
- [ ] Procedural camera bob — velocity-driven offset or `UCameraShakeBase` subclass; active in normal mode only

### Editor
- [ ] `ABP_Arms` Animation Blueprint — states: idle, shift-cast trigger, subtle breathing sway
- [ ] Wire `GameplayTag.Arcane.Vision` → `ABP_Arms` state (shift-cast anim plays on toggle)
- [ ] Blend time on state transitions (suggested: 0.2–0.3s)
- [ ] Assign `ABP_Arms` to `ArmsMesh` in `BP_Hero`
