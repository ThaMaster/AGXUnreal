using UnrealBuildTool;

public class AGXUnrealTests : ModuleRules
{
	public AGXUnrealTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core", "Engine", "CoreUObject", "EditorScriptingUtilities", "AGXUnreal", "AGXUnrealBarrier", "AGXUnrealEditor"
		});
	}
}
