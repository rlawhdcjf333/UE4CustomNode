// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NewProject : ModuleRules
{
	public NewProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "InputCore", "CustomNodeModule"});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
