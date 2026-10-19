---
name: boot
description: Boot the game headless on WorldNull and read the log. Use to check runtime wiring (GameInstance, subsystems, BeginPlay, BP-assigned assets) instead of asking the user to PIE.
allowed-tools: Bash(tasklist*), Bash(powershell.exe -NoProfile -Command *), Bash(grep *)
---

Run in the background:

```bash
powershell.exe -NoProfile -Command "& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject' /Game/Maps/WorldNull -game -nullrhi -unattended -nosplash -log -ExecCmds='quit'"
```

Then grep `Saved/Logs/ThroughArcaneEyes.log` for `LogTae`, `Error` and `Warning`. A healthy boot shows `[GI] Registered HUD viewmodel` and no `LogTae: Warning`. Report each warning with the likely missing BP assignment. See `docs/EngineGotchas.md` for known silent failures.
