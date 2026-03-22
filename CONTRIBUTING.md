# Contributing — Through Arcane Eyes

> AI agents: see [AGENTS.md](AGENTS.md) for conventions in machine-readable form.

---

## Commit Format

```
[TAG][sigil] short description
```

**Sigils:**

| Sigil | Meaning |
|-------|---------|
| `[+]` | New feature or file added |
| `[-]` | Something removed |
| `[*]` | Bug fix, tweak, or small change |

**Tags:**

| Tag | Covers |
|-----|--------|
| `[Core]` | GameMode, GameState, GameInstance |
| `[Character]` | ATaeCharacter, ATaePlayerController |
| `[Input]` | IMC, IA assets, Enhanced Input |
| `[GAS]` | Abilities, AttributeSets, Gameplay Tags |
| `[UI]` | HUD, Widgets, MVVM |
| `[Rendering]` | Materials, Post-Process, Substrate |
| `[Grid]` | GridCube, GridManager |
| `[Animation]` | ABP, Motion Matching, Pose Databases |
| `[Docs]` | README, Roadmap, AGENTS.md |
| `[Config]` | .ini files, Build.cs, .uproject |
| `[Meta]` | License, .gitignore, repo setup |

**Examples:**
```
[Character][+] add first-person camera to ATaeCharacter
[GAS][*] fix ArcaneShift not removing tag on second press
[Config][-] remove unused ModelingToolsEditorMode plugin
[Docs][*] update Day 2 checklist in Roadmap.md
```

Git tags are reserved for sprint/release milestones only (`sprint-1-complete`, `v0.1.0`), not per-commit categorisation.

---

## Code Style

### Naming

| Kind | Convention | Example |
|------|-----------|---------|
| Classes | UE prefix + `Tae` + PascalCase | `ATaeCharacter`, `UTaeGameInstance` |
| Methods | PascalCase | `DoMove`, `BeginPlay` |
| Private members | PascalCase, no underscore | `OwnerCharacter`, `FirstPersonCamera` |
| Input handlers | `Do` prefix | `DoMove`, `DoJump`, `DoToggleEyes` |
| Local variables | camelCase | `Axis`, `EiComp` |

### Properties

Always use `TObjectPtr<T>` for `UPROPERTY` members — never raw `T*`.

| Specifier | When |
|-----------|------|
| `EditDefaultsOnly` | Designer config in BP class defaults |
| `EditAnywhere` | Runtime-assignable (e.g. input actions on controller) |
| `VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true")` | Private component exposed read-only |
| `Transient` | Runtime-only ref cached from an engine callback |
| `BlueprintAssignable` | Delegates exposed to BP |

### Includes

Forward-declare in headers, include the full header in `.cpp` only:

```cpp
// TaePlayerController.h — forward declare
class ATaeCharacter;

// TaePlayerController.cpp — full include
#include "Character/TaeCharacter.h"
```

### Logging

Use `LogTae` (not `LogTemp`). `LogTemp` signals throwaway code.

```cpp
#include "ThroughArcaneEyes.h"
UE_LOG(LogTae, Warning, TEXT("[ClassName] SomeProperty is NULL"));
```

Use `Warning` for null-guards on BP-assigned properties. Do not log normal flow.

### New Classes

1. Header → `Source/ThroughArcaneEyes/Public/<Domain>/TaeFoo.h`
2. Source → `Source/ThroughArcaneEyes/Private/<Domain>/TaeFoo.cpp`
3. `"<Domain>/TaeFoo.generated.h"` must be the **last** include in the header
4. Declare with `THROUGHARCANEEYES_API`

---

## Docs

| File | Update when |
|------|------------|
| [docs/Roadmap.md](docs/Roadmap.md) | Starting or finishing a day; scope changes |
| [docs/Architecture.md](docs/Architecture.md) | Adding a new system or changing data flow |
| [AGENTS.md](AGENTS.md) | A new convention is established that AI agents must follow |
| Relevant `docs/*.md` | Implementing a system that has a design doc |
