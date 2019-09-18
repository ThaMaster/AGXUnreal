using UnrealBuildTool;

public class AGXUnrealBarrier : ModuleRules
{
	public AGXUnrealBarrier(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrecompileForTargets = PrecompileTargetsType.Any;
		bUseRTTI = true;
		bEnableExceptions = true;

		// TODO: Determine which of these are really need and why.
		PublicDependencyModuleNames.AddRange(new string[] {
			"RHI", "RenderCore", "Core", "CoreUObject", "Engine", "InputCore"});

		// TODO: Determine which of these are really needed and why.
		// TODO: Why are some modules listed both here and in Public... above?
		PrivateDependencyModuleNames.AddRange(new string[] {
			"RHI", "RenderCore", "Projects", "Json",
			"AGXUnrealLibrary", "Landscape"});
	}
}
