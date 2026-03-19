# System Architecture — Through Arcane Eyes

> For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

## Class Hierarchy

```
UGameInstance
  └── UTaeGameInstance        owns UTaeHudViewModel (Day 4)

AGameModeBase
  └── ATaeGameMode            registers all framework classes in ctor

AGameState
  └── ATaeGameState

AHUD
  └── ATaeHud                 creates + adds MainWidget to viewport in BeginPlay

APlayerController
  └── ATaePlayerController    owns Enhanced Input; caches ATaeCharacter via SetPawn()

ACharacter
  └── ATaeCharacter           first-person camera; movement driven by controller

UActorComponent
  └── UTaeStateComponent      (Day 2) GameplayTagContainer + OnStateChanged delegate

AActor
  └── ATaeGridCube            (Day 3) mesh + StateComponent; toggles collision
  └── ATaeGridManager         (Day 3) spawns grid from DataTable
  └── ATaePortal              (Day 5) SceneCaptureComponent2D → RenderTarget
```

## Module: ThroughArcaneEyes

Single runtime module. `Build.cs` public dependencies:

```
Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG
```

Planned additions:
- `CommonUI`, `ModelViewViewModel` (Day 4)
- `GameplayTags`, `GameplayAbilities` (Day 2, if GAS is adopted)

## Input Layers

| Layer | Mapping Context | Priority | Active when |
|-------|----------------|----------|-------------|
| Base | `IMC_Default` | 0 | Always |
| Arcane | `IMC_Arcane` | 1 | Spectral Shift toggled on |

Contexts are pushed/popped on `UEnhancedInputLocalPlayerSubsystem` by `ATaePlayerController`.
Input actions are referenced via `UTaeInputConfig` (Data Asset) — no string lookups in C++.

## Framework Wiring

`ATaeGameMode` constructor is the single source of truth for framework class registration:

```cpp
GameStateClass        = ATaeGameState::StaticClass();
HUDClass              = ATaeHud::StaticClass();
DefaultPawnClass      = ATaeCharacter::StaticClass();
PlayerControllerClass = ATaePlayerController::StaticClass();
```

`DefaultGame.ini` overrides this with `BP_TaeGameMode_C` once the Blueprint is created.

## Data Flow: Spectral Shift

```
PlayerInput (IA_SpectralShift)
  → ATaePlayerController::DoSpectralShift()
    → ATaeCharacter::UTaeStateComponent::AddTag(GameplayTag.Arcane.Vision)
      → OnStateChanged.Broadcast()
        → ATaeGridCube (toggles mesh/collision)
        → UTaeHudViewModel (sets bArcaneActive → WBP_HUD updates)
        → Post-Process Volume (enables M_SpectralEdge)
```

## UI Stack

```
Viewport
  └── WBP_HUD                    always present; MVVM-bound, no Tick
  └── WBP_PauseMenu              CommonUI activatable; pushed on pause input
  └── WBP_VictoryScreen          CommonUI activatable; pushed on win condition
```
