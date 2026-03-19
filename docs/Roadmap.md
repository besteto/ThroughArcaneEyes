# Development Roadmap — Through Arcane Eyes

> For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

5-day sprint to a complete portfolio vertical slice.

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 1 | Core Framework | C++ Character/Controller + Enhanced Input | ✅ Done |
| Day 2 | Spectral Shaders | Post-Process Material + C++ Toggle Logic | ⬜ Not started |
| Day 3 | Grid & Interaction | `AGridCube` + hidden/solid state logic | ⬜ Not started |
| Day 4 | Data-Driven UI | MVVM Viewmodel + Common UI HUD | ⬜ Not started |
| Day 5 | Portal & Polish | Render-to-Texture Portal + Win Condition UI | ⬜ Not started |

---

## Day 1 — Core Framework ✅

**Goal:** Playable character in-editor with all input wired end-to-end.

- [x] C++ framework classes — `ATaeCharacter`, `ATaePlayerController`, `ATaeGameMode` (`AGameMode`), `ATaeGameState`, `UTaeGameInstance`, `ATaeHud`
- [x] Enhanced Input — Move / Look / Jump / `DoToggleEyes` stub bound in controller; WASD Swizzle/Negate modifiers correct
- [x] `LogTae` project log category; null-guard warnings on all BP-assigned properties
- [x] Blueprint parenting — `BP_TaeGameMode`, `BP_Hero`, `BP_TaePlayerController` created with correct parents
- [x] Input assets — `IMC_Default`, `IA_MoveInputAction`, `IA_LookInputAction`, `IA_JumpInputAction`
- [x] Config — `GlobalDefaultGameMode` and Enhanced Input component class set

**Remaining loose ends before Day 2:**
- [ ] `IA_ToggleEyesAction` asset — create and assign in `BP_TaePlayerController`
- [ ] `BP_TaeGameState`, `BP_TaeHud` (assign `MainWidgetClass`), `BP_TaeGameInstance` — not strictly required to play but needed before Day 4
- [ ] `DefaultGame.ini` `GameInstanceClass` → `BP_TaeGameInstance_C` once BP is created

---

## Day 2 — Spectral Shaders

**Goal:** Pressing a key toggles the Arcane vision mode; hidden geometry becomes visible.

### C++
- [ ] `UTaeStateComponent` — `UActorComponent` that owns a `FGameplayTagContainer`; broadcasts an `OnStateChanged` delegate
- [ ] `DoToggleEyes` in `ATaePlayerController` — implement push/pop of Arcane `UInputMappingContext` via `UEnhancedInputLocalPlayerSubsystem`; fires `OnStateChanged` tag event

### Materials / Rendering
- [ ] Post-Process Material `M_SpectralEdge` — Sobel-edge detection algorithm
- [ ] Custom Depth Stencil value convention (e.g. `1` = hidden geometry, `2` = portal)
- [ ] `BP_SpectralVolume` — Post-Process Volume that activates `M_SpectralEdge` on demand

### Editor
- [ ] `GA_SpectralShift` Gameplay Ability (or tag) — `GameplayTag.Arcane.Vision` registered in `DefaultGameplayTags.ini`

---

## Day 3 — Grid & Interaction

**Goal:** Procedural cube grid loads; cubes toggle hidden/solid via the Spectral state.

### C++
- [ ] `ATaeGridCube` — Actor with `UStaticMeshComponent` + `UTaeStateComponent`; responds to `OnStateChanged`
- [ ] `ATaeGridManager` — spawns an N×N×N grid of `ATaeGridCube` from a `DataTable` or procedural config
- [ ] Collision toggle — `ATaeGridCube` disables `ECollisionEnabled::NoCollision` when hidden, restores on Arcane off

### Editor
- [ ] `DT_GridLayout` DataTable (row struct `FTaeGridRow`) — defines which cubes start hidden
- [ ] `BP_GridCube` → parent `ATaeGridCube`; assign mesh + materials
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
