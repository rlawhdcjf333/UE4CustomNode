// Copyright DevJongcohol

using UnrealBuildTool;

public class CustomNodeModule : ModuleRules
{
	public CustomNodeModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "BlueprintGraph", "KismetCompiler", "UnrealEd" });
	}
}