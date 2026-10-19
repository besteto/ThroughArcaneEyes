// Copyright © 2026 Helen Allien Poe. See LICENSE.

using UnrealBuildTool;
using System.Collections.Generic;

public class ThroughArcaneEyesTarget : TargetRules
{
	public ThroughArcaneEyesTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("ThroughArcaneEyes");
	}
}
