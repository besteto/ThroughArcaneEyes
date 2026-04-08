// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ThroughArcaneEyes : ModuleRules
{
	public ThroughArcaneEyes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// Engine core
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		// Input
		PublicDependencyModuleNames.AddRange(new string[] { "EnhancedInput" });

		// UI
		PublicDependencyModuleNames.AddRange(new string[] { "UMG", "CommonUI", "ModelViewViewModel" });

		// GAS
		PublicDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "GameplayTasks" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
