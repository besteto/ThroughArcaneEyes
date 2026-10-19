---
name: test
description: Run the project's automation tests headless. Use after building, to verify C++ changes.
allowed-tools: Bash(tasklist*), Bash(powershell.exe -NoProfile -Command *), Bash(grep *)
---

Build first if C++ changed since the last build (see the `compile` skill). The argument is an optional test-name filter; default `ThroughArcaneEyes` runs all, e.g. `ThroughArcaneEyes.GAS.ManaExhaustion` runs one.

Run in the background:

```bash
powershell.exe -NoProfile -Command "& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject' -ExecCmds='Automation RunTests <filter>; Quit' -unattended -nopause -nullrhi -nosplash -log"
```

Then read results from `Saved/Logs/ThroughArcaneEyes.log`: grep for `Test Completed` and `Error`. Report pass/fail counts and the failing test names with their error lines.
