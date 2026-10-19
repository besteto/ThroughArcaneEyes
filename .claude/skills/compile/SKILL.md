---
name: compile
description: Build the ThroughArcaneEyesEditor target with UBT. Use after C++ changes, before running tests or a headless boot.
allowed-tools: Bash(tasklist*), Bash(powershell.exe -NoProfile -Command *)
---

1. Run `tasklist | grep -i UnrealEditor`. If the editor is running, stop and ask the user to close it — UBT refuses to build while Live Coding is active.
2. Run in the background (takes ~1–5 min), writing the full log to the scratchpad:

```bash
powershell.exe -NoProfile -Command "& 'D:\EpicGames\UE_5.8\Engine\Build\BatchFiles\Build.bat' ThroughArcaneEyesEditor Win64 Development -Project='D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject' -WaitMutex $ARGUMENTS"
```

Pass `-DisableUnity` as the argument for a non-unity check before merging a branch.

3. Report `Result: Succeeded` or the first `error` lines with file:line. Don't paste the whole log.
