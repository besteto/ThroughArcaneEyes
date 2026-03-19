# AGENTS.md — ThroughArcaneEyes

Guidance for AI agents working in this UE 5.7 first-person puzzle game.

> For deeper design context see [docs/Architecture.md](docs/Architecture.md) · [docs/Roadmap.md](docs/Roadmap.md) · [docs/SpectralVision.md](docs/SpectralVision.md) · [docs/UIArchitecture.md](docs/UIArchitecture.md)

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

**UPROPERTY specifier patterns in use:**

| Specifier | When used |
|---|---|
| `EditDefaultsOnly` | Designer config set in BP class defaults (e.g. `MainWidgetClass`) |
| `EditAnywhere` | Runtime-assignable in controller (e.g. Input actions/mapping context) |
| `VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true")` | Private component exposed read-only |
| `Transient` | Runtime-only reference cached from engine callback (e.g. `OwnerCharacter`) |

Forward-declare classes used only as `TObjectPtr` in headers; include the actual header in `.cpp`.

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
void DoToggleEyes(IA_t Action);   // ETriggerEvent::Started — Spectral Shift stub (Day 2)
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

Project log category: `LogTae` — declared in `Public/ThroughArcaneEyes.h`, defined in `Private/ThroughArcaneEyes.cpp`.

```cpp
#include "ThroughArcaneEyes.h"
UE_LOG(LogTae, Warning, TEXT("[PC] SomeProperty is NULL — assign it in BP_X"));
```

Use `Warning` for null-guards on BP-assigned properties (fires once at startup, actionable).
Do not use `Display` for normal flow. Do not use `LogTemp` — it signals throwaway code.

---

## Build & Iteration

- **Full rebuild:** open `ThroughArcaneEyes.uproject` in UE5 and use *Tools → Compile* or run UBT from the engine's `Build.bat`.
- **Live Coding:** enabled by default in UE5.7; use `Ctrl+Alt+F11` in editor.
- **Rider:** open the `.uproject`-generated `.sln`; use the built-in UBT run configurations.
- After adding a new `.h`/`.cpp` pair, run *Tools → Refresh Visual Studio Project* (or Rider equivalent) to regenerate project files.

---

## Adding New Classes

1. Place header in `Source/ThroughArcaneEyes/Public/<Domain>/TaeFoo.h`
2. Place implementation in `Source/ThroughArcaneEyes/Private/<Domain>/TaeFoo.cpp`
3. Include `"<Domain>/TaeFoo.generated.h"` as the **last** include in the header
4. Use `THROUGHARCANEEYES_API` in the class declaration
5. No new Build.cs modules needed — everything lives in the single `ThroughArcaneEyes` module
