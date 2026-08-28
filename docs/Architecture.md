# System Architecture — Through Arcane Eyes

> For coding conventions and patterns see [AGENTS.md](../AGENTS.md)
> For the design behind these systems see [the connection loop spec](superpowers/specs/2026-08-10-connection-loop-design.md) and [the restoration economy spec](superpowers/specs/2026-08-11-restoration-economy-design.md)

## The Shape Of It

Three subsystems, deliberately kept apart:

**GAS owns the verbs.** Toggling Arcane Vision and channelling a root are abilities; mana is an attribute; drain and regen are Gameplay Effects. Anything the player *does* is a `UGameplayAbility`.

**Plain actors own the world.** A root connection is a spline, a float, and an enum. It has no Ability System Component and does not need one — giving it a full GAS stack would buy nothing and cost a lot. `ATaeRootPath` owns its own growth state; `ATaeWorldManager` is a registry that counts and broadcasts.

**One subsystem owns the transition.** `UTaeArcaneSubsystem` holds a single `ArcaneBlendAlpha` that drives the post-process weight, the camera blend, and (in M3) the Slate overlay. Three consumers, one interpolator, so their timings cannot drift apart.

## Class Hierarchy

```
UGameInstance
  └── UTaeGameInstance         owns UTaeHudViewModel; publishes it to the MVVM
                               global collection under HudViewModelContextName

UTickableWorldSubsystem
  └── UTaeArcaneSubsystem      ArcaneBlendAlpha interpolator; finds the post-process
                               volume at OnWorldBeginPlay; crossfades music

AGameMode
  └── ATaeGameMode             registers all framework classes in ctor

AGameState
  └── ATaeGameState

AHUD
  └── ATaeHud                  owns every widget; MainMenu + HUD eager,
                               PauseMenu + VictoryScreen lazy

APlayerController
  └── ATaePlayerController     owns Enhanced Input; wires ASC → viewmodel in SetPawn()

ACharacter : IAbilitySystemInterface
  └── ATaeCharacter            ASC + UTaeManaAttributeSet; spring-arm over-shoulder
                               camera; grants SpectralShift + GrowRoot on BeginPlay

UAttributeSet
  └── UTaeManaAttributeSet     Mana / MaxMana; exhaustion hysteresis

UGameplayEffect
  └── UTaeManaEffectBase       periodic, 0.1s; per-period magnitude via SetByCaller
        ├── UTaeManaDrainEffect
        └── UTaeManaRegenEffect

UGameplayAbility
  └── UTaeGameplayAbility      abstract base; AbilityDuration (0 = infinite)
        ├── UGA_SpectralShift  grants Arcane.Vision; pushes IMC_Arcane; drains mana
        └── UGA_GrowRoot       channel at an anchor; advances growth, drains mana

UGameplayCueNotify_Static
  └── UTaeCueNotify_ManaDrain / _ManaRegen / _ArcaneExhausted

UMVVMViewModelBase
  └── UTaeHudViewModel         logic fields + pre-resolved presentation fields

UActorComponent
  └── UTaeStateComponent       listens for Arcane.Vision on the relevant ASC;
                               broadcasts OnArcaneStateChanged

UBoxComponent
  └── UTaeGroveComponent       area-scaled mana regen while occupied

AActor
  └── ATaeGridCube             mesh + StateComponent; swaps material, toggles collision
  └── ATaeGridManager          spawns a grid from a DataTable (unused — islands are hand-placed)
  └── ATaePortal               SceneCaptureComponent2D → RenderTarget
  └── ATaeRootPath             spline + spline-mesh segments; GrowthAlpha + ETaeConnectionState
  └── ATaeRootAnchor           sphere trigger; names the path it grows
  └── ATaeWorldManager         registry over root paths; counts restored, broadcasts
  └── ATaeClutterScatter       randomized small-prop scatter
  └── ATaeInteractableSpawner  semi-random chest/pickup placement at designer markers
```

## Module: ThroughArcaneEyes

Single runtime module, `UseExplicitOrSharedPCHs`. `Build.cs` public dependencies:

| Group | Modules |
|---|---|
| Engine core | `Core`, `CoreUObject`, `Engine`, `InputCore` |
| Input | `EnhancedInput` |
| UI | `UMG`, `CommonUI`, `ModelViewViewModel` |
| GAS | `GameplayAbilities`, `GameplayTags`, `GameplayTasks` |
| Cameras | `GameplayCameras` |

Editor-side work (asset creation, parameter collections) is scripted through `PythonScriptPlugin` rather than adding an editor module. `Niagara` arrives with the arcane presentation pass.

`PoseSearch` was evaluated and dropped — see the note in the [README](../README.md#-technology-stack).

## Gameplay Tags

All tags are native (`UE_DEFINE_GAMEPLAY_TAG` in `TaeGASTypes.cpp`) — there are no `FName` tag strings anywhere in C++.

| Tag | Meaning |
|---|---|
| `Arcane.Vision` | Arcane Vision is active. Granted by `UGA_SpectralShift`. |
| `Arcane.Growing` | A root channel is in progress. |
| `Arcane.Exhausted` | Mana hit zero. Blocks Arcane Vision until mana recovers past the floor. |
| `GameplayCue.Mana.Drain` / `.Regen` | Mana flow cues, driving HUD feedback. |
| `GameplayCue.Arcane.Exhausted` | One-shot cue on exhaustion. |
| `Data.ManaRate` | `SetByCaller` key carrying the per-second rate into the periodic effects. |

## Input Layers

| Layer | Mapping Context | Priority | Active when |
|-------|----------------|----------|-------------|
| Base | `IMC_Default` | 0 | Always |
| Arcane | `IMC_Arcane` | 1 | Arcane Vision active |

Contexts are pushed and popped on `UEnhancedInputLocalPlayerSubsystem` — the base layer by `ATaePlayerController::BeginPlay`, the Arcane layer by `UGA_SpectralShift` itself, so the context's lifetime is exactly the ability's.

All input lives in the controller, never in the character. Handlers use the `Do` prefix: `DoMove`, `DoLook`, `DoJump`, `DoSpectralShift`, `DoGrowRoot`, `DoPause`. Input assets are `TObjectPtr<UInputAction>` properties validated by `IsDataValid`, so a missing binding is an editor error rather than a silent runtime no-op.

## Framework Wiring

`ATaeGameMode`'s constructor is the single source of truth for framework class registration:

```cpp
GameStateClass        = ATaeGameState::StaticClass();
HUDClass              = ATaeHud::StaticClass();
DefaultPawnClass      = ATaeCharacter::StaticClass();
PlayerControllerClass = ATaePlayerController::StaticClass();
```

`DefaultEngine.ini` then overrides `GlobalDefaultGameMode` and `GameInstanceClass` with the Blueprint subclasses.

> **Gotcha worth knowing:** if a class path in `DefaultEngine.ini` fails to resolve, UE falls back to the engine base class *silently* — no error, no warning. A `GameInstanceClass` pointing at a moved asset means `UTaeGameInstance::Init` never runs and everything downstream of it is quietly dead. Verify with a headless run, not by reading the ini.

## Data Flow: Arcane Vision

```
IA_SpectralShift
  → ATaePlayerController::DoSpectralShift()
    → ASC::TryActivateAbility(SpectralShiftHandle)
      → UGA_SpectralShift::ActivateAbility
        ├── grants Arcane.Vision
        ├── pushes IMC_Arcane
        ├── applies UTaeManaDrainEffect (ArcaneDrainPerSecond via Data.ManaRate)
        └── UTaeArcaneSubsystem::SetArcaneActive(true)
              → ArcaneBlendAlpha interpolates over ArcaneTransitionDuration
                ├── post-process volume weight
                ├── camera blend to the pulled-back Arcane framing
                └── music crossfade Forest → Arcane

        Arcane.Vision tag change fans out independently:
          → UTaeStateComponent::OnArcaneStateChanged
            ├── ATaeGridCube      swaps material, toggles collision
            └── ATaeRootPath      reveals ghost segments
          → UTaeHudViewModel::SetArcaneActive → WBP_HUD
```

## Data Flow: Growing A Root

```
IA_GrowRoot (held)
  → ATaePlayerController::DoGrowRoot()
    → UGA_GrowRoot::CanActivateAbility   requires an anchor in range, not Exhausted
      → ActivateAbility
        ├── grants Arcane.Growing
        ├── applies UTaeManaDrainEffect (ManaCostPerSecond)
        └── timer every GrowthTickInterval:
              ATaeRootPath::AdvanceGrowth(GrowthRate * Interval)
                → FTaeGrowthStep::Advance   clamps to [0,1]
                → FTaeGrowthStep::StateFor  Broken → Growing → Restored
                → on state change only: OnConnectionStateChanged
                    → ATaeWorldManager::RecountRestored → OnNetworkChanged

  Release, or mana exhaustion, ends the ability. GrowthAlpha persists.
```

Growth is permanent and partial by design — release early, come back later, resume where you left off. Mana pressure interrupts; it never rolls anything back.

## Data Flow: The Mana Economy

Mana is one attribute with three flows, all of them periodic Infinite Gameplay Effects carrying their rate through a `SetByCaller` on `Data.ManaRate`:

| Flow | Source | Rate |
|---|---|---|
| Vision drain | `UGA_SpectralShift` | `ArcaneDrainPerSecond` (4/s default) |
| Growth drain | `UGA_GrowRoot` | `ManaCostPerSecond` (12/s default) |
| Grove regen | `UTaeGroveComponent` | `Curve_GroveRegen` sampled by footprint area |

Magnitudes are **per period**, not per second — `UTaeManaEffectBase::MagnitudePerPeriod` does the conversion so designers tune in seconds and GAS receives what it actually applies.

Exhaustion has hysteresis. `UTaeManaAttributeSet::EvaluateExhaustion` grants `Arcane.Exhausted` at zero and clears it only once mana climbs back to `RecoveryFraction` of maximum (25% default), so the player cannot flicker in and out of vision at the bottom of the bar. It is a static, ASC-free function precisely so both thresholds are covered by one test and cannot drift apart.

`Arcane.Exhausted` blocks Arcane Vision through a `UTargetTagRequirementsGameplayEffectComponent`, which **inhibits** the grove's regen effect rather than removing it — so regen resumes on leaving Arcane without needing to re-enter the volume.

## UI Stack

```
Viewport
  └── WBP_HUD             always present; MVVM-bound, no Tick
  └── WBP_PauseMenu       CommonUI activatable; lazy, created on first pause
  └── WBP_WinScreen       CommonUI activatable; lazy, created on win
  └── WBP_MainMenu        CommonUI activatable; eager, shown on BeginPlay
```

`ATaeHud` owns all four. Nothing else creates or destroys a widget; UI orchestration goes through `TogglePauseMenu`, `ShowVictoryScreen`, and `ShowMainMenu`.

`UTaeHudViewModel` carries two kinds of field. The **logic** fields (`Mana`, `MaxMana`, `bArcaneActive`, `bExhausted`, `ManaFlow`) are the state. The **presentation** fields (`ManaText`, `ManaPercent`, `ArcaneVisibility`, `ManaBarTint`, `ExhaustedVisibility`) are pre-resolved into the exact types the widget binds to, so View Bindings need no converter functions and the widget stays free of logic.

Mana flow is *counted*, not toggled — vision and growth can drain at once, and the cue notifies are stateless — so `BeginManaFlow`/`EndManaFlow` maintain counters that `ResolveManaFlow` collapses into one displayed state, with draining outranking regenerating.

The viewmodel is owned by `UTaeGameInstance` and published to the MVVM global collection; `WBP_HUD` fetches it with Creation Type **Global View Model Collection**. This matters: the default creation type is `CreateInstance`, which makes the widget silently construct its own viewmodel that nothing ever writes to.

## Testing

Pure game logic is covered by UE Automation Tests running headlessly — no PIE, no editor. The pattern throughout is to push decisions into static, world-free functions so they can be tested directly:

| Function | What it decides |
|---|---|
| `UTaeArcaneSubsystem::StepBlendAlpha` | transition interpolation |
| `FTaeGrowthStep::Advance` / `StateFor` | growth clamping and state thresholds |
| `UTaeManaAttributeSet::EvaluateExhaustion` | exhaustion entry and recovery |
| `UTaeManaEffectBase::MagnitudePerPeriod` | per-second → per-period conversion |
| `UTaeGroveComponent::RegenRateForArea` | area-scaled regen curve lookup |
| `UTaeHudViewModel::ResolveManaFlow` | drain/regen counter resolution |

Where a test genuinely needs a ticking world — the mana drain rate, for one — `Tests/TaeTestWorld.h` provides a scoped world that is created and torn down per test.

Run the suite headlessly:

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" `
  -ExecCmds="Automation RunTests ThroughArcaneEyes; Quit" `
  -unattended -nopause -nullrhi -nosplash -log
```

For a single test, replace `ThroughArcaneEyes` in `RunTests` with the full test name — for example `ThroughArcaneEyes.GAS.ManaExhaustion`.
