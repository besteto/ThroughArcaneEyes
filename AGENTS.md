# AGENTS.md — ThroughArcaneEyes

Guidance for AI agents working in this UE 5.7 first-person puzzle game.

> For deeper design context see [docs/Architecture.md](docs/Architecture.md) · [docs/Roadmap.md](docs/Roadmap.md) · [docs/SpectralVision.md](docs/SpectralVision.md) · [docs/UIArchitecture.md](docs/UIArchitecture.md)
> For human-readable code style and full commit tag vocabulary see [CONTRIBUTING.md](CONTRIBUTING.md)

---

## Module Architecture

Single game module: `ThroughArcaneEyes` (Runtime, `UseExplicitOrSharedPCHs`).

**Build.cs public dependencies (grouped):**
- Engine core: `Core, CoreUObject, Engine, InputCore`
- Input: `EnhancedInput`
- UI: `UMG, CommonUI`
- GAS: `GameplayAbilities, GameplayTags, GameplayTasks`

No custom plugins. Engine plugins in use: `ModelingToolsEditorMode` (editor-only), `GameplayAbilities`, `CommonUI`.

Targets: `ThroughArcaneEyes.Target.cs` (Game) and `ThroughArcaneEyesEditor.Target.cs` (Editor), both `BuildSettingsVersion.V6` / `IncludeOrderVersion.Unreal5_7`.

---

## Naming Conventions

| Kind | Prefix | Example |
|---|---|---|
| Actor subclass | `A` + `Tae` | `ATaeCharacter`, `ATaeGameMode` |
| UObject/Component | `U` + `Tae` | `UTaeGameInstance`, `UCharacterInfo` |
| HUD | `A` + `Tae` | `ATaeHud` |
| Structs | `F` | `FInputActionInstance` |
| API macro | `THROUGHARCANEEYES_API` | `class THROUGHARCANEEYES_API ATaeCharacter` |
| Class prefix alias | `ThisClass` | used in `BindAction` callbacks |

Source mirrors header layout: `Public/<Domain>/TaeX.h` → `Private/<Domain>/TaeX.cpp`.

Current domains: `Core/`, `Character/`, `Data/`, `UI/`, `GAS/`, `Components/`.

---

## Memory Management & Macros

Always use `TObjectPtr<T>` for `UPROPERTY` members — never raw `T*`:

```cpp
// Correct
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
TObjectPtr<UCameraComponent> FirstPersonCamera;

// Wrong
UCameraComponent* FirstPersonCamera;
```

Forward-declare in headers, include in `.cpp`. Full specifier guide in [CONTRIBUTING.md](CONTRIBUTING.md).

---

## Enhanced Input

Input is handled exclusively in `ATaePlayerController` — **not** in `ATaeCharacter`.

```cpp
// TaePlayerController.h — all handlers use const FInputActionInstance& directly
void DoMove(const FInputActionInstance& Action);         // ETriggerEvent::Triggered
void DoLook(const FInputActionInstance& Action);         // ETriggerEvent::Triggered
void DoJump(const FInputActionInstance& Action);         // ETriggerEvent::Started
void DoStopJumping(const FInputActionInstance& Action);  // ETriggerEvent::Completed
void DoSpectralShift(const FInputActionInstance& Action); // ETriggerEvent::Started — toggles UGA_SpectralShift
```

Actions are `EditAnywhere TObjectPtr<UInputAction>` properties on the controller; assigned in `BP_TaePlayerController` Details > Tae.

Mapping context registered in `BeginPlay` via `UEnhancedInputLocalPlayerSubsystem`.
`OwnerCharacter` is cached in `SetPawn()` as a `Transient TObjectPtr<ATaeCharacter>` and null-checked before use.

**IMC modifier conventions:**

| Input source | Action | Modifiers |
|---|---|---|
| W | Move | Swizzle (YXZ) |
| S | Move | Swizzle (YXZ) → Negate |
| A | Move | Negate |
| D | Move | *(none)* |
| Mouse XY 2D-Axis | Look | *(none)* |
| Gamepad Left Thumbstick 2D-Axis | Move | Dead Zone (Axial, 0.2) |
| Gamepad Right Thumbstick 2D-Axis | Look | Dead Zone (Axial, 0.2) → Scalar (2.5, 2.5) |

When adding new input: create a `UInputAction` asset, add it as an `EditAnywhere` property, bind in `SetupInputComponent`, add a null-guard warning using `LogTae`.

---

## Blueprint / C++ Interaction

No `BlueprintNativeEvent` or `BlueprintImplementableEvent` is used yet. Current pattern:

- C++ classes are the **parents** of all Blueprints (`BP_TaeGameMode`, `BP_Hero`, etc.)
- Blueprint-assignable config uses `TSubclassOf<T>` or `TObjectPtr<T>` with `EditDefaultsOnly`/`EditAnywhere`
- `ATaeHud::MainMenuClass` (`TSubclassOf<UTaeMainMenuWidget>`) is set in `BP_TaeHud`; `BeginPlay` creates, `AddToPlayerScreen`, and `ActivateWidget`
- `BP_TaeGameMode` Class Defaults own all framework class slots (`GameStateClass`, `HUDClass`, etc.) — the C++ constructor sets nothing

Delegates in use: `FOnArcaneStateChanged` on `UTaeStateComponent` (`DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam`, `BlueprintAssignable`). Follow this pattern for all future delegates.

---

## Camera

`ATaeCharacter` owns a single `UCameraComponent` (`FirstPersonCamera`):
- Attached to root component
- `SetRelativeLocation(FVector(0, 0, 60))` — eye height
- `bUsePawnControlRotation = true`

---

## GameMode / Framework Wiring

`ATaeGameMode` extends `AGameMode` (not `AGameModeBase`) — required because `ATaeGameState` extends `AGameState`. Mixing these is a hard error at PIE startup.

All four class slots (`GameStateClass`, `HUDClass`, `DefaultPawnClass`, `PlayerControllerClass`) are set in **`BP_TaeGameMode` Class Defaults**, not in C++. The C++ constructor is empty.

`DefaultEngine.ini` `[GameMapsSettings]` points `GlobalDefaultGameMode` to `BP_TaeGameMode_C` and `GameInstanceClass` to `BP_TaeGameInstance_C`.

---

## GAS Conventions

`ATaeCharacter` owns the `UAbilitySystemComponent` and `UTaeManaAttributeSet` (both created as subobjects in the constructor). `InitAbilityActorInfo(this, this)` and `GiveAbility` are called in `BeginPlay`.

**Naming split — Shift vs Vision:**
- *Shift* = the action → `GA_SpectralShift`, `DoSpectralShift`, `IA_SpectralShift`
- *Vision* = the resulting state → `Arcane.Vision` tag, `IMC_Arcane`
- Future shifts (e.g. `GA_EternalShift`) also grant `Arcane.Vision` ± extra tags on top.

**Toggle pattern in `ATaePlayerController::DoSpectralShift`:**
Check `Arcane.Vision` tag → if present: `FindAbilitySpecFromClass` + `CancelAbility`; if absent: `TryActivateAbilityByClass`.

**Attribute sets:** add new `UAttributeSet` subclasses to `Public/GAS/`. Include `GAS/TaeGASTypes.h` for the `ATTRIBUTE_ACCESSORS` macro — never redefine it.

**`UTaeStateComponent`:** add to any actor that needs to react to `Arcane.Vision`. It registers the tag event in `BeginPlay` and broadcasts `OnArcaneStateChanged(bool)`.

---

## Editor Validation

Prefer `IsDataValid` (guarded by `#if WITH_EDITOR`) over runtime `UE_LOG` null-guards for BP-assigned properties. Include `Misc/DataValidation.h` in the `.cpp`. Use `Context.AddError` (not `AddWarning`) for properties that make the class non-functional when unset.

---

## Copyright

All source files use:
```
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.
```
License: Source Available — code is public for portfolio/study only; no redistribution or commercial use. See `LICENSE` in the repo root.

---

## Logging

Use `LogTae` (never `LogTemp`). `Warning` for null-guards only — no flow logging.

```cpp
#include "ThroughArcaneEyes.h"
UE_LOG(LogTae, Warning, TEXT("[PC] SomeProperty is NULL — assign it in BP_X"));
```

---

## Commit Format

```
[TAG][sigil] short description
```

Sigils: `[+]` add · `[-]` remove · `[*]` fix/tweak
Tags: `[Core]` `[Character]` `[Input]` `[GAS]` `[UI]` `[Rendering]` `[Grid]` `[Animation]` `[Docs]` `[Config]` `[Meta]`
Full vocabulary in [CONTRIBUTING.md](CONTRIBUTING.md).

---

## Build & Iteration

- **Full rebuild:** open `ThroughArcaneEyes.uproject` in UE5 and use *Tools → Compile* or run UBT from the engine's `Build.bat`.
- **Live Coding:** enabled by default in UE5.7; use `Ctrl+Alt+F11` in editor.
- **Rider:** open the `.uproject`-generated `.sln`; use the built-in UBT run configurations.
- After adding a new `.h`/`.cpp` pair, run *Tools → Refresh Visual Studio Project* (or Rider equivalent) to regenerate project files.

---

## Adding New Classes

See [CONTRIBUTING.md](CONTRIBUTING.md). Key constraint: no new Build.cs modules — everything lives in the single `ThroughArcaneEyes` module.
