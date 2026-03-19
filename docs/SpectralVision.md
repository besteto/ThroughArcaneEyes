# Spectral Vision System

> Day 2 deliverable. Nothing in this file is implemented yet. For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

## Concept

Pressing the Spectral Shift input toggles "Arcane Eyes" mode. In this mode:
- A Sobel-edge post-process pass outlines all world geometry.
- Hidden geometry (Custom Depth Stencil = 1) becomes visible and gains collision.
- The HUD mana bar drains while the mode is active.

## Gameplay Tag Convention

| Tag | Meaning |
|-----|---------|
| `GameplayTag.Arcane.Vision` | Arcane mode is currently active |
| `GameplayTag.Arcane.Hidden` | Actor is hidden outside Arcane mode |

Register tags in `Config/DefaultGameplayTags.ini`.

## Custom Depth Stencil Values

| Value | Actor type |
|-------|-----------|
| `0` | Default (no stencil) |
| `1` | Hidden geometry (visible only in Arcane mode) |
| `2` | Portal surface |

Set `r.CustomDepth=3` in `DefaultEngine.ini` to enable stencil writes.

## UTaeStateComponent

Attach to any Actor that needs to react to the Spectral state.

```cpp
// Planned API
UCLASS()
class THROUGHARCANEEYES_API UTaeStateComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChanged, FGameplayTag, Tag);

    UPROPERTY(BlueprintAssignable)
    FOnStateChanged OnStateChanged;

    void AddState(FGameplayTag Tag);
    void RemoveState(FGameplayTag Tag);
    bool HasState(FGameplayTag Tag) const;

private:
    UPROPERTY()
    FGameplayTagContainer ActiveStates;
};
```

## Post-Process Material (M_SpectralEdge)

- Scene texture input: `PostProcessInput0`
- Sobel kernel samples 3×3 neighbourhood in UV space
- Output: lerp between normal scene colour and edge-highlight colour based on Sobel magnitude
- Bound to a `BP_SpectralVolume` Post-Process Volume; enabled/disabled via `ATaePlayerController`

## ATaeGridCube State Logic

```
OnStateChanged(Tag):
  if Tag == GameplayTag.Arcane.Vision && Actor has GameplayTag.Arcane.Hidden:
    SetActorHiddenInGame(false)
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)
  else:
    SetActorHiddenInGame(true)
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision)
```
