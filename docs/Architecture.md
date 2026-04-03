# System Architecture — Through Arcane Eyes

> For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

## Class Hierarchy

```
UGameInstance
  └── UTaeGameInstance        owns UTaeHudViewModel (Day 4)

AGameMode
  └── ATaeGameMode            registers all framework classes in ctor

AGameState
  └── ATaeGameState

AHUD
  └── ATaeHud                 creates + adds MainWidget to viewport in BeginPlay

APlayerController
  └── ATaePlayerController    owns Enhanced Input; caches ATaeCharacter via SetPawn()

ACharacter
  └── ATaeCharacter           close over-shoulder camera (SpringArm + FollowCamera); movement driven by controller

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
- `GameplayAbilities`, `GameplayTags`, `GameplayTasks` (Day 2)
- `CommonUI`, `ModelViewViewModel` (Day 4)
- `PoseSearch` (Day 6)

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
PlayerInput (IA_ToggleEyesAction)
  → ATaePlayerController::DoSpectralShift()
    → ATaeCharacter::UAbilitySystemComponent::TryActivateAbility(UGA_ArcaneShift)
      → UGA_ArcaneShift grants/removes GameplayTag.Arcane.Vision
        → UTaeStateComponent::RegisterGameplayTagEvent fires OnStateChanged
          → ATaeGridCube (swaps material, toggles collision)
          → UTaeHudViewModel (sets bArcaneActive → WBP_HUD updates)
          → BP_SpectralVolume (enables M_SpectralEdge post-process)
          → UCharacterMovementComponent (gravity/air control tuned)
```

## UI Stack

```
Viewport
  └── WBP_HUD                    always present; MVVM-bound, no Tick
  └── WBP_PauseMenu              CommonUI activatable; pushed on pause input
  └── WBP_VictoryScreen          CommonUI activatable; pushed on win condition
```
