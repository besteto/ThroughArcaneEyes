# AGENTS.md — ThroughArcaneEyes

Guidance for AI agents working in this UE 5.7 first-person puzzle game.

> For deeper design context see [docs/Architecture.md](docs/Architecture.md) · [docs/Roadmap.md](docs/Roadmap.md) · [docs/SpectralVision.md](docs/SpectralVision.md) · [docs/UIArchitecture.md](docs/UIArchitecture.md)
> For human-readable code style and full commit tag vocabulary see [CONTRIBUTING.md](CONTRIBUTING.md)

---

## Module Architecture

Single game module: `ThroughArcaneEyes` (Runtime, `UseExplicitOrSharedPCHs`).

**Build.cs public dependencies:** `Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG`

No custom plugins. Engine plugin in use: `ModelingToolsEditorMode` (editor-only).

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
// TaePlayerController.h — typedef used for all binding callbacks
using IA_t = const struct FInputActionInstance&;

void DoMove(IA_t Action);         // ETriggerEvent::Triggered
void DoLook(IA_t Action);         // ETriggerEvent::Triggered
void DoJump(IA_t Action);         // ETriggerEvent::Started
void DoStopJumping(IA_t Action);  // ETriggerEvent::Completed
void DoSpectralShift(IA_t Action); // ETriggerEvent::Started — activates UGA_ArcaneShift (Day 2)
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
- `ATaeHud::MainWidgetClass` is set in `BP_HUD`; the C++ `BeginPlay` creates and adds the widget

No delegates are defined yet. When adding delegates, follow UE5 `DECLARE_DYNAMIC_MULTICAST_DELEGATE` convention and expose via `BlueprintAssignable`.

---

## Camera

`ATaeCharacter` owns a single `UCameraComponent` (`FirstPersonCamera`):
- Attached to root component
- `SetRelativeLocation(FVector(0, 0, 60))` — eye height
- `bUsePawnControlRotation = true`

---

## GameMode / Framework Wiring

`ATaeGameMode` extends `AGameMode` (not `AGameModeBase`) — required because `ATaeGameState` extends `AGameState`. Mixing these is a hard error at PIE startup.

Constructor sets all four class slots:

```cpp
GameStateClass        = ATaeGameState::StaticClass();
HUDClass              = ATaeHud::StaticClass();
DefaultPawnClass      = ATaeCharacter::StaticClass();
PlayerControllerClass = ATaePlayerController::StaticClass();
```

`DefaultGame.ini` points `GlobalDefaultGameMode` to `BP_TaeGameMode_C`. When adding new GameMode features, edit the C++ class — the BP inherits them automatically.

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
