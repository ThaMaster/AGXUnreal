using UnrealBuildTool;

public class AGXUnrealEditor : ModuleRules
{
	public AGXUnrealEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrecompileForTargets = PrecompileTargetsType.Any;

		/// \todo Copied from the prototype plugin. Not sure if all of these are
		///       required.
		PublicDependencyModuleNames.AddRange(new string[]{
			"AGXUnreal", "ComponentVisualizers", "Core", "CoreUObject", "Engine", "InputCore", "RHI", "RenderCore"});

		/// \todo Copied from the prototype plugin. Not sure if all of these are
		///       required.
		PrivateDependencyModuleNames.AddRange(new string[] {
			"AGXUnrealBarrier", "AGXUnrealLibrary", "AssetTools", "CoreUObject", "DesktopPlatform", "EditorStyle",
			"Engine", "InputCore", "Json", "LevelEditor", "PlacementMode", "Projects", "PropertyEditor", "RawMesh",
			"RenderCore", "RHI", "Slate", "SlateCore", "SlateCore", "UnrealEd", "SceneOutliner"
		});
    }
}
