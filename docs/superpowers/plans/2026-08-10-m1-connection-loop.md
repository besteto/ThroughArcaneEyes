# M1 — Connection Loop Playable — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the core loop playable — enter Arcane Vision, see a broken root connection, hold to grow it while mana drains, release early and keep partial progress, return and finish, then walk across it in Forest mode.

**Architecture:** State lives on `ATaeRootPath` (`EConnectionState` + `GrowthAlpha`); `ATaeWorldManager` is a registry that counts and broadcasts. GAS owns the verb (`UGA_GrowRoot` inheriting `UTaeGameplayAbility`) because mana cost and cancellation already live there. A single `ArcaneBlendAlpha` on `UTaeArcaneSubsystem` drives post-process, camera, and (later) the overlay so they cannot disagree on timing.

**Tech Stack:** UE 5.8 C++, GAS (`GameplayAbilities`), Enhanced Input, `GameplayCameras`, UE Automation Tests. No `PoseSearch`, no `UAF/*`, no `Mover` — all experimental in 5.8.

**Source spec:** [`docs/superpowers/specs/2026-08-10-connection-loop-design.md`](../specs/2026-08-10-connection-loop-design.md) §3, §4, §6, §11 M1.

## Global Constraints

- Engine: UE 5.8 at `D:\EpicGames\UE_5.8`. Project: `D:\PetProjects\ThroughArcaneEyes`.
- Single runtime module `ThroughArcaneEyes`. (Slate/editor modules are now permitted per spec §13 but M1 needs none.)
- Every file starts with: `// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.`
- Naming: `A`+`Tae` actors, `U`+`Tae` objects/components, `F` structs, `E` enums. API macro `THROUGHARCANEEYES_API`.
- **All `UPROPERTY` members use `TObjectPtr<T>`, never raw `T*`.** Forward-declare in headers, include in `.cpp`.
- Logging: `LogTae` only, never `LogTemp`. `Warning` for null-guards only, no flow logging.
- Prefer `IsDataValid` under `#if WITH_EDITOR` over runtime null-guards for BP-assigned properties. Include `Misc/DataValidation.h` in the `.cpp`.
- Gameplay tags declared in `Public/GAS/TaeGASTypes.h`, defined in `Private/GAS/TaeGASTypes.cpp` via `UE_DEFINE_GAMEPLAY_TAG`. **No `FName` tag strings in calling code.**
- All input handled in `ATaePlayerController`, never in `ATaeCharacter`. Handlers use the `Do` prefix mirroring the IA name.
- Source mirrors headers: `Public/<Domain>/TaeX.h` → `Private/<Domain>/TaeX.cpp`.
- Commit format: `[TAG][sigil] short description`. Sigils `[+]` add, `[-]` remove, `[*]` fix/tweak. Tags used here: `[World]`, `[GAS]`, `[Core]`, `[Input]`, `[Config]`, `[Meta]`, `[Docs]`.

**Build command** (run after every code change):

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Build\BatchFiles\Build.bat' ThroughArcaneEyesEditor Win64 Development -Project="D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -WaitMutex
```
Expected on success: `Result: Succeeded`.

**Test command:**

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -ExecCmds="Automation RunTests ThroughArcaneEyes; Quit" -unattended -nopause -nullrhi -nosplash -log
```
Expected on success: log contains `Test Completed. Result={Passed}` and no `Result={Failed}`.

---

## File Structure

| File | Responsibility |
|---|---|
| `Public/World/TaeConnectionTypes.h` | **Create.** `EConnectionState` enum + `FTaeGrowthStep` pure growth math. No actor dependencies, so it is unit-testable without a world. |
| `Private/World/TaeConnectionTypes.cpp` | **Create.** `FTaeGrowthStep::Advance` implementation. |
| `Private/Tests/TaeConnectionTypesTest.cpp` | **Create.** Automation tests for growth math. |
| `Private/Tests/TaeBlendAlphaTest.cpp` | **Create.** Automation tests for the Arcane blend interpolation. |
| `Public/World/TaeRootPath.h` / `.cpp` | **Modify.** Add state, `GrowthAlpha`, `AdvanceGrowth`, `OnConnectionStateChanged`. Drive spline mesh reveal from growth, not just the Arcane tag. |
| `Public/World/TaeRootAnchor.h` / `.cpp` | **Create.** Marker actor at each end of a path; the volume the player must overlap to channel. |
| `Public/World/TaeWorldManager.h` / `.cpp` | **Modify.** Registry: collect paths, count restored, broadcast `OnNetworkChanged`. |
| `Public/GAS/GA_GrowRoot.h` / `.cpp` | **Create.** The channel ability — mana drain per second, advances the target path. |
| `Public/GAS/TaeGASTypes.h` / `.cpp` | **Modify.** Add `TAG_Arcane_Growing`. |
| `Public/Core/TaeArcaneSubsystem.h` / `.cpp` | **Modify.** `UTickableWorldSubsystem`, `ArcaneBlendAlpha`, post-process via `BlendWeight`, vignette interpolation fix. |
| `Public/Character/TaePlayerController.h` / `.cpp` | **Modify.** `IA_GrowRoot` property + `DoGrowRoot` / `DoStopGrowRoot`. |
| `Public/Character/TaeCharacter.h` / `.cpp` | **Modify.** Expose `GrowRootHandle` alongside `SpectralShiftHandle`. |
| `ThroughArcaneEyes.Build.cs` | **Modify.** Add `GameplayCameras`. |

---

## Task 1: Automation test harness

The project has zero tests. Nothing downstream can be TDD'd until a test can run headless. This task's deliverable is a green test run, not a feature.

**Files:**
- Create: `Source/ThroughArcaneEyes/Private/Tests/TaeHarnessTest.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: the working test command in Global Constraints; the `ThroughArcaneEyes.<Domain>.<Thing>` test-name convention used by every later task.

- [ ] **Step 1: Write a deliberately failing test**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeHarnessTest.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeHarnessTest,
	"ThroughArcaneEyes.Harness.Sanity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeHarnessTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("harness is wired up"), 1 + 1, 3);
	return true;
}

#endif
```

> `EAutomationTestFlags` is a scoped enum in 5.8 (`AutomationTest.h:88`). The macro `static_assert`s that exactly one application-context flag and one filter flag are present — `EditorContext | EngineFilter` satisfies both.

- [ ] **Step 2: Build**

Run the build command. Expected: `Result: Succeeded`.

- [ ] **Step 3: Run the test and verify it FAILS**

Run the test command. Expected: log contains `Result={Failed}` and a message showing `2` vs `3`.

This proves the harness actually executes assertions rather than silently passing.

- [ ] **Step 4: Correct the assertion**

```cpp
	TestEqual(TEXT("harness is wired up"), 1 + 1, 2);
```

- [ ] **Step 5: Rebuild and verify it PASSES**

Run build, then the test command. Expected: `Result={Passed}`, no `Result={Failed}`.

- [ ] **Step 6: Commit**

```powershell
git add Source/ThroughArcaneEyes/Private/Tests/TaeHarnessTest.cpp
git commit -m "[Meta][+] add automation test harness"
```

---

## Task 2: Growth math

Pure functions first — no actor, no world, fully testable. Every growth rule the game depends on lives here.

**Files:**
- Create: `Source/ThroughArcaneEyes/Public/World/TaeConnectionTypes.h`
- Create: `Source/ThroughArcaneEyes/Private/World/TaeConnectionTypes.cpp`
- Test: `Source/ThroughArcaneEyes/Private/Tests/TaeConnectionTypesTest.cpp`

**Interfaces:**
- Consumes: Task 1's test conventions.
- Produces:
  - `enum class EConnectionState : uint8 { Broken, Growing, Restored }`
  - `FTaeGrowthStep::Advance(float CurrentAlpha, float DeltaAlpha) -> float` (clamped 0..1)
  - `FTaeGrowthStep::StateFor(float Alpha) -> EConnectionState`

- [ ] **Step 1: Write the failing tests**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeConnectionTypesTest.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "World/TaeConnectionTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeGrowthStepTest,
	"ThroughArcaneEyes.World.GrowthStep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeGrowthStepTest::RunTest(const FString& Parameters)
{
	// Advance accumulates
	TestEqual(TEXT("advance from zero"), FTaeGrowthStep::Advance(0.f, 0.25f), 0.25f);
	TestEqual(TEXT("advance accumulates"), FTaeGrowthStep::Advance(0.25f, 0.25f), 0.5f);

	// Advance clamps at both ends — release-early must never overshoot or go negative
	TestEqual(TEXT("clamps at one"), FTaeGrowthStep::Advance(0.9f, 0.5f), 1.f);
	TestEqual(TEXT("clamps at zero"), FTaeGrowthStep::Advance(0.1f, -0.5f), 0.f);

	// State thresholds
	TestTrue(TEXT("zero is broken"),
		FTaeGrowthStep::StateFor(0.f) == EConnectionState::Broken);
	TestTrue(TEXT("partial is growing"),
		FTaeGrowthStep::StateFor(0.5f) == EConnectionState::Growing);
	TestTrue(TEXT("one is restored"),
		FTaeGrowthStep::StateFor(1.f) == EConnectionState::Restored);

	// Boundary: anything above zero has begun, only exactly-full is restored
	TestTrue(TEXT("epsilon above zero is growing"),
		FTaeGrowthStep::StateFor(KINDA_SMALL_NUMBER * 2.f) == EConnectionState::Growing);
	TestTrue(TEXT("just under one is still growing"),
		FTaeGrowthStep::StateFor(0.999f) == EConnectionState::Growing);

	return true;
}

#endif
```

- [ ] **Step 2: Run tests to verify they fail**

Run the build command. Expected: FAIL — `Cannot open include file: 'World/TaeConnectionTypes.h'`.

- [ ] **Step 3: Write the header**

Create `Source/ThroughArcaneEyes/Public/World/TaeConnectionTypes.h`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "TaeConnectionTypes.generated.h"

// Lifecycle of one root connection. Growth is permanent and partial — a path that reaches Growing
// never falls back to Broken on its own, only when explicitly reset.
UENUM(BlueprintType)
enum class EConnectionState : uint8
{
	Broken     UMETA(DisplayName = "Broken"),
	Growing    UMETA(DisplayName = "Growing"),
	Restored   UMETA(DisplayName = "Restored")
};

// Pure growth rules, deliberately free of actor/world dependencies so they can be tested directly.
struct THROUGHARCANEEYES_API FTaeGrowthStep
{
	// Adds DeltaAlpha to CurrentAlpha, clamped to [0,1].
	static float Advance(float CurrentAlpha, float DeltaAlpha);

	// Maps an alpha to its connection state.
	static EConnectionState StateFor(float Alpha);
};
```

- [ ] **Step 4: Write the implementation**

Create `Source/ThroughArcaneEyes/Private/World/TaeConnectionTypes.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeConnectionTypes.h"

float FTaeGrowthStep::Advance(const float CurrentAlpha, const float DeltaAlpha)
{
	return FMath::Clamp(CurrentAlpha + DeltaAlpha, 0.f, 1.f);
}

EConnectionState FTaeGrowthStep::StateFor(const float Alpha)
{
	if (Alpha >= 1.f)
	{
		return EConnectionState::Restored;
	}
	if (Alpha > KINDA_SMALL_NUMBER)
	{
		return EConnectionState::Growing;
	}
	return EConnectionState::Broken;
}
```

- [ ] **Step 5: Build and run tests**

Run build, then the test command. Expected: `ThroughArcaneEyes.World.GrowthStep` → `Result={Passed}`.

- [ ] **Step 6: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/World/TaeConnectionTypes.h Source/ThroughArcaneEyes/Private/World/TaeConnectionTypes.cpp Source/ThroughArcaneEyes/Private/Tests/TaeConnectionTypesTest.cpp
git commit -m "[World][+] add connection state and growth math"
```

---

## Task 3: Root path growth state

`ATaeRootPath` currently reveals on the Arcane tag alone. Now reveal is driven by growth: segments appear progressively as the root grows, and restored segments stay visible in Forest mode.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/World/TaeRootPath.h`
- Modify: `Source/ThroughArcaneEyes/Private/World/TaeRootPath.cpp`

**Interfaces:**
- Consumes: `EConnectionState`, `FTaeGrowthStep` from Task 2.
- Produces:
  - `ATaeRootPath::AdvanceGrowth(float DeltaAlpha) -> void`
  - `ATaeRootPath::GetGrowthAlpha() const -> float`
  - `ATaeRootPath::GetConnectionState() const -> EConnectionState`
  - `FOnConnectionStateChanged` (`DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams`, `ATaeRootPath*`, `EConnectionState`) exposed as `OnConnectionStateChanged`

- [ ] **Step 1: Add state to the header**

In `TaeRootPath.h`, after the existing includes add `#include "World/TaeConnectionTypes.h"`, and above the `UCLASS()`:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConnectionStateChanged, ATaeRootPath*, Path, EConnectionState, NewState);
```

In the `public:` section, after `OnConstruction`:

```cpp
	// Advances growth by DeltaAlpha (negative shrinks). Clamped 0..1; broadcasts only on state change.
	void AdvanceGrowth(float DeltaAlpha);

	float GetGrowthAlpha() const { return GrowthAlpha; }
	EConnectionState GetConnectionState() const { return ConnectionState; }

	UPROPERTY(BlueprintAssignable, Category = "RootPath")
	FOnConnectionStateChanged OnConnectionStateChanged;
```

In the `private:` section, alongside the existing properties:

```cpp
	// Persists across Arcane toggles — growth is permanent and partial
	UPROPERTY(VisibleAnywhere, Category = "RootPath")
	float GrowthAlpha = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "RootPath")
	EConnectionState ConnectionState = EConnectionState::Broken;

	// Applies GrowthAlpha and Arcane state to segment visibility/collision
	void RefreshSegments();
```

Also change the existing private helper declaration from `void SetSegmentsRevealed(bool bRevealed);` to nothing — `RefreshSegments` replaces it. Add a cached flag beside the others:

```cpp
	bool bArcaneActive = false;
```

- [ ] **Step 2: Write the failing behaviour into the implementation contract**

Replace `SetSegmentsRevealed` and `OnArcaneStateChanged` in `TaeRootPath.cpp` with:

```cpp
void ATaeRootPath::AdvanceGrowth(const float DeltaAlpha)
{
	const float NewAlpha = FTaeGrowthStep::Advance(GrowthAlpha, DeltaAlpha);
	if (FMath::IsNearlyEqual(NewAlpha, GrowthAlpha))
	{
		return;
	}

	GrowthAlpha = NewAlpha;

	const EConnectionState NewState = FTaeGrowthStep::StateFor(GrowthAlpha);
	const bool bStateChanged = NewState != ConnectionState;
	ConnectionState = NewState;

	RefreshSegments();

	if (bStateChanged)
	{
		OnConnectionStateChanged.Broadcast(this, ConnectionState);
	}
}

void ATaeRootPath::OnArcaneStateChanged(const bool bInArcaneActive)
{
	bArcaneActive = bInArcaneActive;
	RefreshSegments();
}

void ATaeRootPath::RefreshSegments()
{
	const int32 NumSegments = SplineMeshSegments.Num();
	if (NumSegments == 0)
	{
		return;
	}

	// Segments materialise in order as the root grows
	const float GrownSegments = GrowthAlpha * static_cast<float>(NumSegments);

	for (int32 Index = 0; Index < NumSegments; ++Index)
	{
		USplineMeshComponent* Segment = SplineMeshSegments[Index];
		if (!Segment)
		{
			continue;
		}

		const bool bGrown = static_cast<float>(Index) < GrownSegments;

		// Grown segments are solid in both modes; ungrown ones are ghosts only Arcane can see
		const bool bVisible = bGrown || bArcaneActive;
		const bool bCollides = bGrown;

		Segment->SetVisibility(bVisible);
		Segment->SetCollisionEnabled(bCollides
			? ECollisionEnabled::QueryAndPhysics
			: ECollisionEnabled::NoCollision);
	}
}
```

Add `#include "World/TaeConnectionTypes.h"` to the `.cpp` includes.

In `BeginPlay`, replace the `SetSegmentsRevealed(false)` call with `RefreshSegments();`.

Update the `OnArcaneStateChanged` declaration in the header to take `bool bInArcaneActive` so the parameter name matches.

- [ ] **Step 3: Build**

Run the build command. Expected: `Result: Succeeded`.

- [ ] **Step 4: Verify in PIE**

1. Open the project. Place a `BP_RootPath` (create it in Task 8 if absent — for now place the C++ class directly) with a 4-point spline and any mesh assigned to `PathMesh`.
2. PIE. Expected: no segments visible in Forest mode.
3. Toggle Arcane. Expected: all 4 segments visible, none walkable.
4. In the console run `GetAll TaeRootPath GrowthAlpha`. Expected: `0.0`.

- [ ] **Step 5: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/World/TaeRootPath.h Source/ThroughArcaneEyes/Private/World/TaeRootPath.cpp
git commit -m "[World][*] drive root path reveal from growth state"
```

---

## Task 4: Root anchors

The player must stand somewhere specific to channel. Anchors are that somewhere, and they name which path they feed.

**Files:**
- Create: `Source/ThroughArcaneEyes/Public/World/TaeRootAnchor.h`
- Create: `Source/ThroughArcaneEyes/Private/World/TaeRootAnchor.cpp`

**Interfaces:**
- Consumes: `ATaeRootPath` from Task 3.
- Produces:
  - `ATaeRootAnchor::GetPath() const -> ATaeRootPath*`
  - `ATaeRootAnchor::GetGrowthDirection() const -> float` (`+1` grows forward, `-1` backward)

- [ ] **Step 1: Write the header**

Create `Source/ThroughArcaneEyes/Public/World/TaeRootAnchor.h`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TaeRootAnchor.generated.h"

class ATaeRootPath;
class USphereComponent;

// The spot Ant stands to grow a root. One at each end of an ATaeRootPath; the player overlaps it and
// channels UGA_GrowRoot. Placed by a designer next to the island edge.
UCLASS()
class THROUGHARCANEEYES_API ATaeRootAnchor : public AActor
{
	GENERATED_BODY()

public:
	ATaeRootAnchor();

	ATaeRootPath* GetPath() const { return Path; }
	float GetGrowthDirection() const { return bGrowsForward ? 1.f : -1.f; }

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	UPROPERTY(VisibleAnywhere, Category = "RootAnchor")
	TObjectPtr<USphereComponent> ChannelRange;

	// The connection this anchor grows — assign in the level
	UPROPERTY(EditInstanceOnly, Category = "RootAnchor")
	TObjectPtr<ATaeRootPath> Path;

	// Anchors at the spline start grow forward; anchors at the far end grow backward
	UPROPERTY(EditInstanceOnly, Category = "RootAnchor")
	bool bGrowsForward = true;
};
```

- [ ] **Step 2: Write the implementation**

Create `Source/ThroughArcaneEyes/Private/World/TaeRootAnchor.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeRootAnchor.h"
#include "World/TaeRootPath.h"
#include "Components/SphereComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ATaeRootAnchor::ATaeRootAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	ChannelRange = CreateDefaultSubobject<USphereComponent>(TEXT("ChannelRange"));
	SetRootComponent(ChannelRange);
	ChannelRange->SetSphereRadius(200.f);
	ChannelRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ChannelRange->SetCollisionResponseToAllChannels(ECR_Overlap);
}

#if WITH_EDITOR
EDataValidationResult ATaeRootAnchor::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!Path)
	{
		Context.AddError(FText::FromString(TEXT("Path is not set — assign the ATaeRootPath this anchor grows")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
```

- [ ] **Step 3: Build**

Run the build command. Expected: `Result: Succeeded`.

- [ ] **Step 4: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/World/TaeRootAnchor.h Source/ThroughArcaneEyes/Private/World/TaeRootAnchor.cpp
git commit -m "[World][+] add root anchor marker"
```

---

## Task 5: World manager registry

`ATaeWorldManager` is currently an empty class with an unused private array. This gives it its job.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/World/TaeWorldManager.h`
- Modify: `Source/ThroughArcaneEyes/Private/World/TaeWorldManager.cpp`

**Interfaces:**
- Consumes: `ATaeRootPath::OnConnectionStateChanged`, `GetConnectionState()` from Task 3.
- Produces:
  - `ATaeWorldManager::GetRestoredCount() const -> int32`
  - `ATaeWorldManager::GetRequiredCount() const -> int32`
  - `FOnNetworkChanged` (`DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams`, `int32 Restored`, `int32 Required`) exposed as `OnNetworkChanged`

- [ ] **Step 1: Replace the header**

Replace the body of `TaeWorldManager.h` with:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/TaeConnectionTypes.h"
#include "TaeWorldManager.generated.h"

class ATaeRootPath;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkChanged, int32, RestoredCount, int32, RequiredCount);

// Per-level registry of root connections. Each ATaeRootPath still owns its own state; this actor
// subscribes to them, keeps the counts, and is the single thing the HUD and win condition observe.
UCLASS()
class THROUGHARCANEEYES_API ATaeWorldManager : public AActor
{
	GENERATED_BODY()

public:
	ATaeWorldManager();

	int32 GetRestoredCount() const { return RestoredCount; }
	int32 GetRequiredCount() const { return RootPaths.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "World")
	FOnNetworkChanged OnNetworkChanged;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	UFUNCTION()
	void HandleConnectionStateChanged(ATaeRootPath* Path, EConnectionState NewState);

	void RecountRestored();

	UPROPERTY(EditInstanceOnly, Category = "World")
	TArray<TObjectPtr<ATaeRootPath>> RootPaths;

	UPROPERTY(VisibleAnywhere, Category = "World")
	int32 RestoredCount = 0;
};
```

- [ ] **Step 2: Replace the implementation**

Replace the body of `TaeWorldManager.cpp` with:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeWorldManager.h"
#include "World/TaeRootPath.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ATaeWorldManager::ATaeWorldManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATaeWorldManager::BeginPlay()
{
	Super::BeginPlay();

	for (const TObjectPtr<ATaeRootPath>& Path : RootPaths)
	{
		if (Path)
		{
			Path->OnConnectionStateChanged.AddDynamic(this, &ATaeWorldManager::HandleConnectionStateChanged);
		}
	}

	RecountRestored();
	OnNetworkChanged.Broadcast(RestoredCount, GetRequiredCount());
}

void ATaeWorldManager::HandleConnectionStateChanged(ATaeRootPath* Path, EConnectionState NewState)
{
	RecountRestored();
	OnNetworkChanged.Broadcast(RestoredCount, GetRequiredCount());
}

void ATaeWorldManager::RecountRestored()
{
	RestoredCount = 0;
	for (const TObjectPtr<ATaeRootPath>& Path : RootPaths)
	{
		if (Path && Path->GetConnectionState() == EConnectionState::Restored)
		{
			++RestoredCount;
		}
	}
}

#if WITH_EDITOR
EDataValidationResult ATaeWorldManager::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (RootPaths.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("RootPaths is empty — assign the ATaeRootPath actors for this level")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
```

- [ ] **Step 3: Build**

Run the build command. Expected: `Result: Succeeded`.

- [ ] **Step 4: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/World/TaeWorldManager.h Source/ThroughArcaneEyes/Private/World/TaeWorldManager.cpp
git commit -m "[World][*] make world manager a real connection registry"
```

---

## Task 6: Arcane blend alpha

Post-process is a hard on/off switch today. The camera blend in Task 8 and the overlay in M3 both need a shared 0..1 or they will visibly disagree. The interpolation itself is a pure function so it can be tested.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/Core/TaeArcaneSubsystem.h`
- Modify: `Source/ThroughArcaneEyes/Private/Core/TaeArcaneSubsystem.cpp`
- Test: `Source/ThroughArcaneEyes/Private/Tests/TaeBlendAlphaTest.cpp`

**Interfaces:**
- Consumes: nothing from earlier tasks.
- Produces:
  - `UTaeArcaneSubsystem::StepBlendAlpha(float Current, float Target, float DeltaTime, float Duration) -> float` (static, pure)
  - `UTaeArcaneSubsystem::GetArcaneBlendAlpha() const -> float`

- [ ] **Step 1: Write the failing test**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeBlendAlphaTest.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "Core/TaeArcaneSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeBlendAlphaTest,
	"ThroughArcaneEyes.Core.BlendAlpha",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeBlendAlphaTest::RunTest(const FString& Parameters)
{
	// Half the duration covers half the distance
	TestEqual(TEXT("half step toward one"),
		UTaeArcaneSubsystem::StepBlendAlpha(0.f, 1.f, 0.25f, 0.5f), 0.5f);

	// Never overshoots the target
	TestEqual(TEXT("clamps to target going up"),
		UTaeArcaneSubsystem::StepBlendAlpha(0.9f, 1.f, 1.f, 0.5f), 1.f);
	TestEqual(TEXT("clamps to target going down"),
		UTaeArcaneSubsystem::StepBlendAlpha(0.1f, 0.f, 1.f, 0.5f), 0.f);

	// Already at target is a no-op
	TestEqual(TEXT("at target stays"),
		UTaeArcaneSubsystem::StepBlendAlpha(1.f, 1.f, 0.016f, 0.5f), 1.f);

	// Zero or negative duration snaps rather than dividing by zero
	TestEqual(TEXT("zero duration snaps"),
		UTaeArcaneSubsystem::StepBlendAlpha(0.f, 1.f, 0.016f, 0.f), 1.f);

	return true;
}

#endif
```

- [ ] **Step 2: Run to verify failure**

Run the build command. Expected: FAIL — `StepBlendAlpha` is not a member of `UTaeArcaneSubsystem`.

- [ ] **Step 3: Update the header**

In `TaeArcaneSubsystem.h`, change the include and base class:

```cpp
#include "Subsystems/WorldSubsystem.h"
```
stays, but the class declaration becomes:

```cpp
UCLASS()
class THROUGHARCANEEYES_API UTaeArcaneSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// Called by GA_SpectralShift Activate/End — sets the blend target and crossfades music
	void SetArcaneActive(bool bActive);

	// Spike vignette intensity then fade back; Duration is total fade-out time
	void FlashVignette(float Duration = 0.5f);

	float GetArcaneBlendAlpha() const { return ArcaneBlendAlpha; }

	// Pure interpolation step — moves Current toward Target so a full traverse takes Duration seconds.
	// Duration <= 0 snaps. Static and world-free so it can be tested directly.
	static float StepBlendAlpha(float Current, float Target, float DeltaTime, float Duration);

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void CrossfadeMusic(bool bToArcane);
	void ApplyBlendAlpha();

	UPROPERTY()
	TObjectPtr<APostProcessVolume> SpectralVolume;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ForestMusicComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ArcaneMusicComp;

	// Seconds for a full Forest <-> Arcane traverse. Post-process, camera, and the M3 overlay all
	// read the resulting alpha so their timings cannot drift apart.
	UPROPERTY(EditAnywhere, Category = "Arcane")
	float ArcaneTransitionDuration = 0.35f;

	float ArcaneBlendAlpha = 0.f;
	float ArcaneBlendTarget = 0.f;

	float VignetteFlashAlpha = 0.f;
	float VignetteFlashDuration = 0.5f;
};
```

Remove the now-unused `ClearVignetteFlash` declaration, the `VignetteTimerHandle`, and `bVolumeActive`.

- [ ] **Step 4: Update the implementation**

In `TaeArcaneSubsystem.cpp`, replace `SetArcaneActive`, `FlashVignette`, and `ClearVignetteFlash` with:

```cpp
float UTaeArcaneSubsystem::StepBlendAlpha(const float Current, const float Target, const float DeltaTime, const float Duration)
{
	if (Duration <= 0.f)
	{
		return Target;
	}

	const float Step = DeltaTime / Duration;
	return FMath::FInterpConstantTo(Current, Target, 1.f, Step);
}

void UTaeArcaneSubsystem::SetArcaneActive(const bool bActive)
{
	ArcaneBlendTarget = bActive ? 1.f : 0.f;

	CrossfadeMusic(bActive);
	FlashVignette();
}

void UTaeArcaneSubsystem::FlashVignette(const float Duration)
{
	VignetteFlashDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	VignetteFlashAlpha = 1.f;
}

void UTaeArcaneSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float NewAlpha = StepBlendAlpha(ArcaneBlendAlpha, ArcaneBlendTarget, DeltaTime, ArcaneTransitionDuration);
	const bool bAlphaChanged = !FMath::IsNearlyEqual(NewAlpha, ArcaneBlendAlpha);
	ArcaneBlendAlpha = NewAlpha;

	// Vignette now actually interpolates — it used to snap to zero on a timer
	const bool bFlashing = VignetteFlashAlpha > 0.f;
	if (bFlashing)
	{
		VignetteFlashAlpha = FMath::Max(VignetteFlashAlpha - (DeltaTime / VignetteFlashDuration), 0.f);
	}

	if (bAlphaChanged || bFlashing)
	{
		ApplyBlendAlpha();
	}
}

TStatId UTaeArcaneSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTaeArcaneSubsystem, STATGROUP_Tickables);
}

void UTaeArcaneSubsystem::ApplyBlendAlpha()
{
	if (!SpectralVolume)
	{
		return;
	}

	// BlendWeight replaces the old binary bEnabled so the volume fades with the camera
	SpectralVolume->bEnabled = ArcaneBlendAlpha > 0.f;
	SpectralVolume->BlendWeight = ArcaneBlendAlpha;

	FPostProcessSettings& Settings = SpectralVolume->Settings;
	Settings.bOverride_VignetteIntensity = true;
	Settings.VignetteIntensity = VignetteFlashAlpha;
}
```

In `OnWorldBeginPlay`, replace `SpectralVolume->bEnabled = false;` with:

```cpp
		SpectralVolume->bEnabled = false;
		SpectralVolume->BlendWeight = 0.f;
```

- [ ] **Step 5: Build and run tests**

Run build, then the test command. Expected: `ThroughArcaneEyes.Core.BlendAlpha` → `Result={Passed}`.

- [ ] **Step 6: Verify in PIE**

PIE and toggle Arcane. Expected: post-process now **fades** over ~0.35s rather than popping, and the vignette flash fades smoothly instead of snapping off.

- [ ] **Step 7: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/Core/TaeArcaneSubsystem.h Source/ThroughArcaneEyes/Private/Core/TaeArcaneSubsystem.cpp Source/ThroughArcaneEyes/Private/Tests/TaeBlendAlphaTest.cpp
git commit -m "[Core][*] blend arcane post-process on a shared alpha"
```

---

## Task 7: GrowRoot ability and input

The verb. GAS owns it because mana cost and cancellation already live in `UTaeGameplayAbility`.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/GAS/TaeGASTypes.h`
- Modify: `Source/ThroughArcaneEyes/Private/GAS/TaeGASTypes.cpp`
- Create: `Source/ThroughArcaneEyes/Public/GAS/GA_GrowRoot.h`
- Create: `Source/ThroughArcaneEyes/Private/GAS/GA_GrowRoot.cpp`
- Modify: `Source/ThroughArcaneEyes/Public/Character/TaeCharacter.h`, `Private/Character/TaeCharacter.cpp`
- Modify: `Source/ThroughArcaneEyes/Public/Character/TaePlayerController.h`, `Private/Character/TaePlayerController.cpp`

**Interfaces:**
- Consumes: `ATaeRootAnchor::GetPath()`, `GetGrowthDirection()` (Task 4); `ATaeRootPath::AdvanceGrowth` (Task 3).
- Produces: `TAG_Arcane_Growing`; `ATaeCharacter::GrowRootHandle`; `ATaePlayerController::DoGrowRoot` / `DoStopGrowRoot`.

- [ ] **Step 1: Add the native tag**

In `TaeGASTypes.h`, beside the existing `TAG_Arcane_Vision` declaration:

```cpp
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Arcane_Growing);
```

In `TaeGASTypes.cpp`, beside the existing definition:

```cpp
UE_DEFINE_GAMEPLAY_TAG(TAG_Arcane_Growing, "Arcane.Growing");
```

- [ ] **Step 2: Write the ability header**

Create `Source/ThroughArcaneEyes/Public/GAS/GA_GrowRoot.h`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GAS/TaeGameplayAbility.h"
#include "GA_GrowRoot.generated.h"

class ATaeRootAnchor;
class ATaeRootPath;

// Hold-to-channel root growth. Activates only while Arcane.Vision is active and the avatar overlaps an
// ATaeRootAnchor. Drains mana per second and advances that anchor's path. Growth is permanent — ending
// early leaves the path partially grown.
UCLASS()
class THROUGHARCANEEYES_API UGA_GrowRoot : public UTaeGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_GrowRoot();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	// Finds the nearest overlapping anchor on the avatar, or nullptr
	ATaeRootAnchor* FindAnchorInRange(const FGameplayAbilityActorInfo* ActorInfo) const;

	void TickGrowth();

	// Alpha added per second of channelling — a full path takes 1/GrowthRate seconds
	UPROPERTY(EditDefaultsOnly, Category = "GrowRoot")
	float GrowthRate = 0.35f;

	// Mana drained per second while channelling
	UPROPERTY(EditDefaultsOnly, Category = "GrowRoot")
	float ManaCostPerSecond = 12.f;

	// How often growth is applied. Coarser than frame rate; growth is not frame-dependent.
	UPROPERTY(EditDefaultsOnly, Category = "GrowRoot")
	float GrowthTickInterval = 0.05f;

	UPROPERTY()
	TObjectPtr<ATaeRootPath> ActivePath;

	float GrowthDirection = 1.f;
	FTimerHandle GrowthTimerHandle;
};
```

- [ ] **Step 3: Write the ability implementation**

Create `Source/ThroughArcaneEyes/Private/GAS/GA_GrowRoot.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "GAS/GA_GrowRoot.h"
#include "GAS/TaeGASTypes.h"
#include "GAS/TaeManaAttributeSet.h"
#include "World/TaeRootAnchor.h"
#include "World/TaeRootPath.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "ThroughArcaneEyes.h"

UGA_GrowRoot::UGA_GrowRoot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(TAG_Arcane_Growing);
}

bool UGA_GrowRoot::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Growing is an Arcane-mode act
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC || !ASC->HasMatchingGameplayTag(TAG_Arcane_Vision))
	{
		return false;
	}

	return FindAnchorInRange(ActorInfo) != nullptr;
}

ATaeRootAnchor* UGA_GrowRoot::FindAnchorInRange(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!Avatar)
	{
		return nullptr;
	}

	TArray<AActor*> Overlapping;
	Avatar->GetOverlappingActors(Overlapping, ATaeRootAnchor::StaticClass());

	ATaeRootAnchor* Nearest = nullptr;
	float NearestDistSq = TNumericLimits<float>::Max();

	for (AActor* Actor : Overlapping)
	{
		ATaeRootAnchor* Anchor = Cast<ATaeRootAnchor>(Actor);
		if (!Anchor || !Anchor->GetPath())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Avatar->GetActorLocation(), Anchor->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = Anchor;
		}
	}

	return Nearest;
}

void UGA_GrowRoot::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ATaeRootAnchor* Anchor = FindAnchorInRange(ActorInfo);
	if (!Anchor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActivePath = Anchor->GetPath();
	GrowthDirection = Anchor->GetGrowthDirection();

	UWorld* World = GetWorld();
	if (!World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	World->GetTimerManager().SetTimer(
		GrowthTimerHandle, this, &UGA_GrowRoot::TickGrowth, GrowthTickInterval, true);
}

void UGA_GrowRoot::TickGrowth()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ActivePath || !ASC)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// Out of mana ends the channel; progress so far is kept
	const float Mana = ASC->GetNumericAttribute(UTaeManaAttributeSet::GetManaAttribute());
	const float ManaThisTick = ManaCostPerSecond * GrowthTickInterval;
	if (Mana < ManaThisTick)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	ASC->SetNumericAttributeBase(UTaeManaAttributeSet::GetManaAttribute(), Mana - ManaThisTick);
	ActivePath->AdvanceGrowth(GrowthRate * GrowthTickInterval * GrowthDirection);

	// Finished — stop rather than burning mana on a full path
	if (ActivePath->GetConnectionState() == EConnectionState::Restored && GrowthDirection > 0.f)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_GrowRoot::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GrowthTimerHandle);
	}
	ActivePath = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
```

> `EConnectionState` reaches this file via `TaeRootPath.h`, which includes `TaeConnectionTypes.h`.

- [ ] **Step 4: Expose the ability on the character**

In `TaeCharacter.h`, beside the existing `SpectralShiftAbility` / `SpectralShiftHandle` members, add matching ones:

```cpp
	UPROPERTY(EditDefaultsOnly, Category = "Tae|Abilities")
	TSubclassOf<class UTaeGameplayAbility> GrowRootAbility;

	FGameplayAbilitySpecHandle GrowRootHandle;
```

In `TaeCharacter.cpp` `BeginPlay`, mirror the existing `GiveAbility` call for `SpectralShiftAbility`, storing the result in `GrowRootHandle` and guarding on `GrowRootAbility` being set with a `LogTae` warning if not.

- [ ] **Step 5: Wire the input**

In `TaePlayerController.h`, beside the existing input action properties:

```cpp
	UPROPERTY(EditAnywhere, Category = "Tae|Input")
	TObjectPtr<UInputAction> IA_GrowRoot;
```

and beside the existing handlers:

```cpp
	void DoGrowRoot(const FInputActionInstance& Action);
	void DoStopGrowRoot(const FInputActionInstance& Action);
```

In `TaePlayerController.cpp` `SetupInputComponent`, bind following the existing pattern:

```cpp
	if (IA_GrowRoot)
	{
		EnhancedInput->BindAction(IA_GrowRoot, ETriggerEvent::Started, this, &ThisClass::DoGrowRoot);
		EnhancedInput->BindAction(IA_GrowRoot, ETriggerEvent::Completed, this, &ThisClass::DoStopGrowRoot);
	}
	else
	{
		UE_LOG(LogTae, Warning, TEXT("[PC] IA_GrowRoot is NULL — assign it in BP_TaePlayerController"));
	}
```

and the handlers:

```cpp
void ATaePlayerController::DoGrowRoot(const FInputActionInstance& Action)
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		ASC->TryActivateAbility(OwnerCharacter->GrowRootHandle);
	}
}

void ATaePlayerController::DoStopGrowRoot(const FInputActionInstance& Action)
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		ASC->CancelAbilityHandle(OwnerCharacter->GrowRootHandle);
	}
}
```

> If `GrowRootHandle` is private on `ATaeCharacter`, add a public getter rather than widening the member — match however `SpectralShiftHandle` is already exposed.

- [ ] **Step 6: Build**

Run the build command. Expected: `Result: Succeeded`.

- [ ] **Step 7: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/GAS Source/ThroughArcaneEyes/Private/GAS Source/ThroughArcaneEyes/Public/Character Source/ThroughArcaneEyes/Private/Character
git commit -m "[GAS][+] add GrowRoot channel ability and input"
```

---

## Task 8: Camera rigs, content, and the M1 gate

Editor work. This is where the loop becomes playable and the milestone gate gets recorded.

**Files:**
- Modify: `Source/ThroughArcaneEyes/ThroughArcaneEyes.Build.cs`
- Modify: `ThroughArcaneEyes.uproject`
- Content: `Content/Character/Input/IA_GrowRoot`, `Content/GAS/BP_GA_GrowRoot`, `Content/World/BP_RootPath`, `BP_RootAnchor`, `BP_WorldManager`, `Content/Maps/WorldNull.umap`

- [ ] **Step 1: Add the GameplayCameras dependency**

In `ThroughArcaneEyes.Build.cs`, extend the UI/camera deps:

```csharp
		// Camera
		PublicDependencyModuleNames.AddRange(new string[] { "GameplayCameras" });
```

In `ThroughArcaneEyes.uproject`, add to the `Plugins` array:

```json
		{
			"Name": "GameplayCameras",
			"Enabled": true
		},
```

- [ ] **Step 2: Build**

Run the build command. Expected: `Result: Succeeded` with no "does not list plugin" warning.

- [ ] **Step 3: Create the input asset**

In the editor, create `Content/Character/Input/IA_GrowRoot` (`UInputAction`, Digital bool). Add it to `IMC_Default` bound to **E** and to Gamepad Face Button Right. Assign it to `IA_GrowRoot` on `BP_TaePlayerController`.

- [ ] **Step 4: Create the ability and world Blueprints**

- `Content/GAS/BP_GA_GrowRoot` — parent `UGA_GrowRoot`. Leave `GrowthRate`, `ManaCostPerSecond`, `GrowthTickInterval` at defaults for now. Assign it to `GrowRootAbility` on `BP_Hero`'s parent Blueprint `BP_TaeCharacter`.
- `Content/World/BP_RootPath` — parent `ATaeRootPath`. Assign any placeholder cylinder to `PathMesh` and `M_GridCube_Arcane` to `PathMaterial`.
- `Content/World/BP_RootAnchor` — parent `ATaeRootAnchor`.
- `Content/World/BP_WorldManager` — parent `ATaeWorldManager`.

- [ ] **Step 5: Build the island pair**

In `WorldNull.umap`:

1. Keep the two existing cube islands as the placeholder art (they are deleted in M4, not now).
2. Place one `BP_RootPath` spanning the gap, 4–6 spline points, sagging slightly.
3. Place `BP_RootAnchor` at each end. Set `Path` on both to that root path. Set `bGrowsForward = true` on the near-island anchor, `false` on the far one.
4. Place one `BP_WorldManager`; add the root path to its `RootPaths` array.
5. Run **Build > Validate Data** — expected: zero errors from `IsDataValid` on all four actors.

- [ ] **Step 6: Add the Arcane camera rig**

Create a GameplayCameras rig asset for the pulled-back Arcane framing and a second for the existing over-shoulder Forest framing. Blend between them on `UTaeArcaneSubsystem::GetArcaneBlendAlpha()`.

> Keep the existing `USpringArmComponent` on `ATaeCharacter` in place until this is proven — spec §9 and §12 both call it the fallback. Only remove it once the blend is working.

- [ ] **Step 7: Run the M1 gate**

PIE and verify each in order:

1. Forest mode — the root path is invisible and you cannot walk across the gap.
2. Toggle Arcane — post-process fades in over ~0.35s, camera pulls back over the same interval, ghost root appears.
3. Stand on the near anchor, hold **E** — segments materialise one by one, mana drains.
4. Release at roughly half — growth stops, the grown half stays solid.
5. Toggle back to Forest — the grown half is still visible and walkable; the ungrown half is gone.
6. Return to Arcane, hold **E** to completion — the ability ends itself at full growth.
7. Toggle to Forest and walk the full connection to the far island.
8. Console `GetAll TaeWorldManager RestoredCount` — expected `1`.

- [ ] **Step 8: Record the gate clip**

Capture steps 2–7 as a single take. This is the M1 deliverable.

- [ ] **Step 9: Commit**

```powershell
git add Source/ThroughArcaneEyes/ThroughArcaneEyes.Build.cs ThroughArcaneEyes.uproject Content
git commit -m "[World][+] add island pair, root path content and arcane camera rig"
```

---

## Self-Review

**Spec coverage (§11 M1):**

| M1 requirement | Task |
|---|---|
| `ATaeRootPath` gains `EConnectionState` + `GrowthAlpha` | 2, 3 |
| `ATaeRootAnchor` marker actor | 4 |
| `ATaeWorldManager` registry with `OnNetworkChanged` | 5 |
| `UGA_GrowRoot` + `IA_GrowRoot` + `DoGrowRoot`, mana drain, partial persistence | 7, 8 |
| `ArcaneBlendAlpha`; post-process on `BlendWeight` | 6 |
| GameplayCameras Forest/Arcane rigs blended on that alpha | 8 |
| One hand-built island pair, placeholder meshes | 8 |
| §6.4 `FlashVignette` interpolation fix | 6 |

**Deliberately out of M1:** the Slate overlay, `UTaeArcanePalette`, chromatic aberration and vignette reproduction in paint (all M3); Control Rig (M2); PCG and the retirement list (M4); win condition and save game (M5).

**Type consistency:** `GrowthAlpha` is the property name in Tasks 3, 7, 8. `AdvanceGrowth` in 3 and 7. `GetConnectionState()` in 3, 5, 7. `StepBlendAlpha` in 6 only. `EConnectionState::Restored` in 3, 5, 7. `GetArcaneBlendAlpha()` produced in 6, consumed in 8.

**Known risk carried into execution:** Task 7 assumes `UTaeManaAttributeSet` exposes `GetManaAttribute()` via the `ATTRIBUTE_ACCESSORS` macro and that `ATaeCharacter::GetAbilitySystemComponent()` is public. Both follow from the existing GAS setup, but the implementer should confirm the exact accessor names in `TaeManaAttributeSet.h` before writing `TickGrowth` rather than assuming the spelling.
