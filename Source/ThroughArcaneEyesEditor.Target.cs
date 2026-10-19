// Copyright © 2026 Helen Allien Poe. See LICENSE.

using UnrealBuildTool;
using System.Collections.Generic;

public class ThroughArcaneEyesEditorTarget : TargetRules
{
	public ThroughArcaneEyesEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("ThroughArcaneEyes");
	}
}
