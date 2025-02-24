// Copyright 2023, Algoryx Simulation AB.


using UnrealBuildTool;

public class AGXShaders : ModuleRules
{
	public AGXShaders(ReadOnlyTargetRules Target) : base(Target)
	{
		bLegacyPublicIncludePaths = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core", "Engine", "UnrealEd", "CoreUObject"
		});

        PublicDependencyModuleNames.AddRange(new string[] {
            "AGXCommon", "AGXUnrealBarrier", "RHI", "RenderCore", "Core", "CoreUObject", "Engine",
            "InputCore", "Niagara"});

        if (Target.bBuildEditor == true)
    {
      PrivateDependencyModuleNames.Add("TargetPlatform");
    }
    PublicDependencyModuleNames.Add("Core");
    PublicDependencyModuleNames.Add("Engine");
    PublicDependencyModuleNames.Add("MaterialShaderQualitySettings");

    PrivateDependencyModuleNames.AddRange(new string[]
    {
      "CoreUObject",
      "Renderer",
      "RenderCore",
      "RHI",
      "Projects",
      "AGXUnrealBarrier"
    });
  }
}
