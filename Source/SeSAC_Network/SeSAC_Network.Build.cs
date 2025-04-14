// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SeSAC_Network : ModuleRules
{
	public SeSAC_Network(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "OnlineSubsystem", "OnlineSubsystemSteam" });
	}
}
