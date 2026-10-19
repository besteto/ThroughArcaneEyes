# AGENTS.md — ThroughArcaneEyes

Guidance for AI agents working in this UE 5.8 third-person (over-the-shoulder) restoration game: Ant, a humanoid tree, grows roots between islands in a ruined world.

> Design context: [docs/Architecture.md](docs/Architecture.md) · [docs/Roadmap.md](docs/Roadmap.md) · [docs/SpectralVision.md](docs/SpectralVision.md) · [docs/UIArchitecture.md](docs/UIArchitecture.md)
> Specs and plans per milestone: [docs/superpowers/](docs/superpowers/) · Engine pitfalls already paid for: [docs/EngineGotchas.md](docs/EngineGotchas.md)
> Code style and full commit tag vocabulary: [CONTRIBUTING.md](CONTRIBUTING.md)

---

## Module Architecture

Single game module: `ThroughArcaneEyes` (Runtime, `UseExplicitOrSharedPCHs`).

**Build.cs public dependencies (grouped):**
- Engine core: `Core, CoreUObject, Engine, InputCore`
- Input: `EnhancedInput`
- UI: `UMG, CommonUI, ModelViewViewModel`
- VFX: `Niagara`
- GAS: `GameplayAbilities, GameplayTags, GameplayTasks`
- Cameras: `GameplayCameras`

No custom plugins. Engine plugins enabled in the `.uproject`: `ModelingToolsEditorMode`, `CommonUI`, `ModelViewViewModel` (+ `Preview`), `GameplayAbilities`, `GameplayCameras`, `PythonScriptPlugin` (editor asset scripting in `Tools/Python/`).

Targets: `ThroughArcaneEyes.Target.cs` (Game) and `ThroughArcaneEyesEditor.Target.cs` (Editor), both `BuildSettingsVersion.V7` / `IncludeOrderVersion.Unreal5_8`.

---

## Naming Conventions

| Kind | Prefix | Example |
|---|---|---|
| Actor subclass | `A` + `Tae` | `ATaeCharacter`, `ATaeGameMode` |
| UObject/Component | `U` + `Tae` | `UTaeGameInstance`, `UTaeGroveComponent` |
| Gameplay Ability | `UGA_` | `UGA_SpectralShift`, `UGA_GrowRoot` |
| HUD | `A` + `Tae` | `ATaeHud` |
| Structs | `F` | `FInputActionInstance` |
| API macro | `THROUGHARCANEEYES_API` | `class THROUGHARCANEEYES_API ATaeCharacter` |
| Class prefix alias | `ThisClass` | used in `BindAction` callbacks |

Name things after game vocabulary (grove, root, growth front), not engine vocabulary. Input handlers use the `Do` prefix; input actions use `IA_`.

Source mirrors header layout: `Public/<Domain>/TaeX.h` → `Private/<Domain>/TaeX.cpp`.

Current domains: `Core/`, `Character/`, `UI/`, `GAS/`, `Components/`, `World/`. Tests live in `Private/Tests/` (see Build & Test). `Data/CharacterInfo.h` is a dead stub — don't build on it.

---

## Memory Management & Macros

Always use `TObjectPtr<T>` for `UPROPERTY` members — never raw `T*`:

```cpp
// Correct
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
TObjectPtr<UCameraComponent> FollowCamera;

// Wrong
UCameraComponent* FollowCamera;
```

Forward-declare in headers, include in `.cpp`. Full specifier guide in [CONTRIBUTING.md](CONTRIBUTING.md).

---

## Enhanced Input

Input is handled exclusively in `ATaePlayerController` — **not** in `ATaeCharacter`.

```cpp
// TaePlayerController.h — all handlers use const FInputActionInstance& directly
void DoMove(const FInputActionInstance& Action);          // Triggered
void DoLook(const FInputActionInstance& Action);          // Triggered
void DoJump(const FInputActionInstance& Action);          // Started
void DoStopJumping(const FInputActionInstance& Action);   // Completed
void DoSpectralShift(const FInputActionInstance& Action); // Started — toggles UGA_SpectralShift
void DoGrowRoot(const FInputActionInstance& Action);      // channel UGA_GrowRoot
void DoStopGrowRoot(const FInputActionInstance& Action);  // end the channel
void DoPause(const FInputActionInstance& Action);         // fires OnPauseRequested
```

Actions are `EditAnywhere TObjectPtr<UInputAction>` properties on the controller; assigned in `BP_TaePlayerController` Details > Tae. Mapping contexts: `IMC_Default`, plus `IMC_Arcane` pushed by `UGA_SpectralShift`.

Mapping context registered in `BeginPlay` via `UEnhancedInputLocalPlayerSubsystem`. `OwnerCharacter` is cached in `SetPawn()` as a `Transient TObjectPtr<ATaeCharacter>` and null-checked before use. `SetPawn()` also wires the ASC to the HUD viewmodel.

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

When adding new input: create a `UInputAction` asset, add it as an `EditAnywhere` property, bind in `SetupInputComponent`, and add an `IsDataValid` error for it (see Editor Validation).

---

## Blueprint / C++ Interaction

- C++ classes are the **parents** of all Blueprints (`BP_TaeGameMode`, `BP_TaeCharacter`, …)
- Blueprint-assignable config uses `TSubclassOf<T>` or `TObjectPtr<T>` with `EditDefaultsOnly`/`EditAnywhere`
- `BlueprintImplementableEvent` is used sparingly — currently only `ATaePlayerController::OnPauseRequested`, which BP routes to `ATaeHud::TogglePauseMenu`
- `ATaeHud` owns all UI orchestration (`TogglePauseMenu`, `ShowVictoryScreen`, `ShowMainMenu`); widgets derive from `UTaeActivatableWidget`
- `UTaeHudViewModel` is owned by `UTaeGameInstance` and published to the MVVM global collection — widgets must fetch it, not create their own (see [docs/EngineGotchas.md](docs/EngineGotchas.md))
- `BP_TaeGameMode` Class Defaults own all framework class slots — the C++ constructor sets nothing

Delegates use `DECLARE_DYNAMIC_MULTICAST_DELEGATE_*` + `BlueprintAssignable`: `FOnArcaneStateChanged` (`UTaeStateComponent`), `FOnConnectionStateChanged` (`ATaeRootPath`), `FOnNetworkChanged` (`ATaeWorldManager`). Follow this pattern.

**Dynamic per-instance components (`ATaeClutterScatter`, `ATaeRootPath`):** when the number of child components depends on per-placement data (a mesh pool, a spline's point count), create them in `OnConstruction` instead of the constructor — `NewObject<T>(this)` → `SetupAttachment` → `RegisterComponent()` → `AddInstanceComponent()`. Destroy and rebuild on every call so editor edits preview live. `CreateDefaultSubobject` remains the default for everything with a fixed component count.

**Niagara hooks:** C++ owns the `UNiagaraComponent` and pushes user parameters; the `NS_` asset is authored by hand. Parameter names are declared as constants in the owning header (`TaeGroveParams` in `TaeGroveComponent.h`, `TaeRootParams` in `TaeRootPath.h`) — keep the asset's user parameters identical. Colours come from `UTaeArcanePalette` → `MPC_Arcane` (mirrored to Niagara via `NPC_Arcane`), never hardcoded.

---

## Camera

`ATaeCharacter` uses a close over-the-shoulder third-person camera:
- `SpringArm` (`USpringArmComponent`) attached to root; `TargetArmLength = 80`, `SocketOffset = (0, 50, 20)`, `bUsePawnControlRotation = true`
- `FollowCamera` (`UCameraComponent`) attached to `SpringArm` socket; `bUsePawnControlRotation = false`
- Defaults are set in C++; fine-tune in `BP_TaeCharacter` Details

---

## GameMode / Framework Wiring

`ATaeGameMode` extends `AGameMode` (not `AGameModeBase`) — required because `ATaeGameState` extends `AGameState`. Mixing these is a hard error at PIE startup.

All four class slots (`GameStateClass`, `HUDClass`, `DefaultPawnClass`, `PlayerControllerClass`) are set in **`BP_TaeGameMode` Class Defaults**, not in C++.

`DefaultEngine.ini` `[GameMapsSettings]` points `GlobalDefaultGameMode` to `BP_TaeGameMode_C` and `GameInstanceClass` to `/Game/Core/BP_TaeGameInstance.BP_TaeGameInstance_C`. If that path is wrong UE **silently** falls back to `UGameInstance` — check the log.

---

## GAS Conventions

`ATaeCharacter` owns the `UAbilitySystemComponent` and `UTaeManaAttributeSet` (both subobjects in the constructor). `BeginPlay` calls `InitAbilityActorInfo(this, this)` and `GiveAbility`, storing `SpectralShiftHandle` and `GrowRootHandle`.

**Naming split — Shift vs Vision:**
- *Shift* = the action → `GA_SpectralShift`, `DoSpectralShift`, `IA_SpectralShift`
- *Vision* = the resulting state → `Arcane.Vision` tag, `IMC_Arcane`
- Future shifts (e.g. `GA_EternalShift`) also grant `Arcane.Vision` ± extra tags on top.

**Toggle pattern in `ATaePlayerController::DoSpectralShift`:** check `Arcane.Vision` → if present `CancelAbilityHandle(SpectralShiftHandle)`, else `TryActivateAbility(SpectralShiftHandle)`.

**Tags:** native tags only (`UE_DEFINE_GAMEPLAY_TAG` in `TaeGASTypes.cpp`) — no `FName` tag strings.

**Mana economy:** drain, regen and growth cost are periodic Infinite Gameplay Effects defined in C++ (`TaeManaEffects`) with SetByCaller magnitudes. `Arcane.Exhausted` ejects from Vision and holds until a recovery floor. Gameplay Cues need a `GC_` BP asset per cue (see gotchas).

**Attribute sets:** add new `UAttributeSet` subclasses to `Public/GAS/`. Include `GAS/TaeGASTypes.h` for the `ATTRIBUTE_ACCESSORS` macro — never redefine it.

**`UTaeStateComponent`:** add to any actor that needs to react to `Arcane.Vision`. It registers the tag event in `BeginPlay` and broadcasts `OnArcaneStateChanged(bool)`.

---

## Editor Validation

Prefer `IsDataValid` (guarded by `#if WITH_EDITOR`) over runtime `UE_LOG` null-guards for BP-assigned properties. Include `Misc/DataValidation.h` in the `.cpp`. Use `Context.AddError` (not `AddWarning`) for properties that make the class non-functional when unset. Early-out with `if (IsTemplate())` so Blueprint templates aren't validated.

---

## Copyright & License

All source files use:
```
// Copyright © 2026 Helen Allien Poe. See LICENSE.
```
Dual licensed: code (`Source/`, `Tools/`, `Config/`, build files) is Apache 2.0 ([LICENSE](LICENSE)); `Content/` and the game's name, story and art are All Rights Reserved ([LICENSE-CONTENT.md](LICENSE-CONTENT.md)). The Fab Stickman asset is not tracked — never commit it.

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
Tags: `[Core]` `[Character]` `[Input]` `[GAS]` `[UI]` `[World]` `[Rendering]` `[Grid]` `[Animation]` `[Docs]` `[Config]` `[Meta]`
Full vocabulary in [CONTRIBUTING.md](CONTRIBUTING.md).

---

## Build & Test

Close the editor first — UBT refuses to build while Live Coding is active, and Live Coding can't pick up **new** source files anyway.

```powershell
# Build the editor target
& 'D:\EpicGames\UE_5.8\Engine\Build\BatchFiles\Build.bat' ThroughArcaneEyesEditor Win64 Development -Project="D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -WaitMutex

# Run all automation tests (single test: use its full name, e.g. ThroughArcaneEyes.GAS.ManaExhaustion)
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -ExecCmds="Automation RunTests ThroughArcaneEyes; Quit" -unattended -nopause -nullrhi -nosplash -log

# Headless game boot — runs GameInstance::Init and world BeginPlay; then read Saved/Logs/ThroughArcaneEyes.log
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" /Game/Maps/WorldNull -game -nullrhi -unattended -nosplash -log -ExecCmds="quit"

# Editor asset scripts
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -run=pythonscript -script="<Tools/Python/script.py>" -unattended -nopause -nosplash
```

Prefer the headless boot over asking the user to PIE when checking runtime wiring. Tests use `IMPLEMENT_SIMPLE_AUTOMATION_TEST`; tests needing a world share `Private/Tests/TaeTestWorld.h`.

After adding a new `.h`/`.cpp` pair, regenerate project files (*Tools → Refresh Visual Studio Project* or Rider equivalent).

---

## Workflow

- Milestone work follows spec → plan → implementation in `docs/superpowers/specs/` and `docs/superpowers/plans/`.
- Manual editor steps go in a handoff note in `docs/issues/` (gitignored scratch between user and agent); promote anything worth keeping into `docs/superpowers/`.
- `docs/ideas/` holds the user's own notes — don't commit them.

---

## Adding New Classes

See [CONTRIBUTING.md](CONTRIBUTING.md). Keep gameplay code in the single `ThroughArcaneEyes` module; separate Slate/editor tooling modules are allowed.
