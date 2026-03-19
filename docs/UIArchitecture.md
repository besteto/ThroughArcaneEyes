# UI Architecture — Through Arcane Eyes

> Day 4 deliverable. Nothing in this file is implemented yet. For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

## Principles

- No `Tick` in any widget.
- No `Event Construct` bindings to game state — use MVVM field notifications only.
- Input focus is managed by Common UI, not manually via `SetInputMode`.

## Widget Hierarchy

| Widget | Base class | Role |
|--------|-----------|------|
| `WBP_HUD` | `UUserWidget` | Persistent HUD; mana bar + vision indicator |
| `WBP_PauseMenu` | `UCommonActivatableWidget` | Pause overlay; auto-captures input |
| `WBP_VictoryScreen` | `UCommonActivatableWidget` | End screen; triggered by win condition |

`ATaeHud::BeginPlay` creates and adds `WBP_HUD`. Pause and Victory screens are pushed onto the Common UI activatable stack by game logic — never by the HUD widget itself.

## UTaeHudViewModel

Owned by `UTaeGameInstance` (persistent across map loads).

```cpp
// Planned fields
UCLASS()
class THROUGHARCANEEYES_API UTaeHudViewModel : public UMVVMViewModelBase
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
    float Mana = 100.f;

    UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
    bool bArcaneActive = false;
};
```

`WBP_HUD` binds to these fields in the UMG Binding panel (One-Way binding, source = ViewModel).
The ViewModel is resolved via `UMVVMGameSubsystem` — no manual `GetGameInstance()` calls in widgets.

## Common UI Input Routing

`WBP_PauseMenu` and `WBP_VictoryScreen` extend `UCommonActivatableWidget`.
- Activate: `ActivateWidget()` (called by game logic)
- Deactivate: back/cancel input handled automatically by Common UI input router
- No `SetInputMode(FInputModeUIOnly{})` needed

Required Build.cs deps: `CommonUI`, `ModelViewViewModel`.

## Adding a New Screen

1. Create `WBP_Foo` based on `UCommonActivatableWidget`.
2. Override `GetDesiredFocusTarget` to return the first focusable button.
3. Call `UCommonUIExtensions::PushContentToLayer_ForPlayer(...)` from C++ to show it.
4. Do not wire game state directly — update the ViewModel and let bindings propagate.
