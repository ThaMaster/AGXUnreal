using UnrealBuildTool;

public class AGXUnrealEditor : ModuleRules
{
	public AGXUnrealEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrecompileForTargets = PrecompileTargetsType.Any;

		// TODO: The prototype plugin has more here.
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "AGXUnreal" });

		// TODO: The prototype plugin has more here.
		PrivateDependencyModuleNames.AddRange(new string[] {
			"CoreUObject", "Engine", "UnrealEd", "Slate", "SlateCore", "PropertyEditor", "EditorStyle", "AGXUnrealBarrier"});
	}
}
