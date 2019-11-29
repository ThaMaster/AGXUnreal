using System.IO;
using System;
using UnrealBuildTool;


/// The AGXUnrealLibrary is the portal from AGXUnrealBarrier to AGX Dynamics.
/// This is where we list the include paths and linker requirements to build an
/// Unreal Engine plugin that uses AGX Dynamics.
public class AGXUnrealLibrary : ModuleRules
{
	/// Information about how AGX Dynamics is packaged and used on the current
	/// platform.
	private PlatformInfo CurrentPlatform;

	public AGXUnrealLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		CurrentPlatform = new PlatformInfo(Target);
		Type = ModuleType.External;

		// Because the AGX Dynamics type system uses typeid and dynamic_cast.
		bUseRTTI = true;

		// Because AGX Dynamics uses exceptions.
		bEnableExceptions = true;

		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath("agxPhysics"));
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath("agxCable"));
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath("agxCore"));
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath("agxHydraulics"));
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath("agxModel"));
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath("agxPhysics"));
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath("agxSabre"));
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath("agxTerrain"));
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath("agxVehicle"));
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkTerrainDependencyLibraryPath("openvdb"));

		// TODO: Do we need to list more libraries here, or will transitive
		// dependencies be enough?.

		RuntimeDependencies.Add(CurrentPlatform.RuntimeLibraryPath("agxPhysics"));
		// TODO: Do we need to list more libraries here, or will transitive
		//       dependencies be enough?
		// TODO: Do we need to add to RuntimeDependencies at all?
		//       Experimentation in the prototype plugin indicate not.

		PublicRuntimeLibraryPaths.Add(CurrentPlatform.RuntimeLibrariesDirectory);
		PublicRuntimeLibraryPaths.Add(CurrentPlatform.TerrainDependenciesRuntimeLibrariesDirectory);

		PublicIncludePaths.Add(CurrentPlatform.LibraryIncludePath);
		PublicIncludePaths.Add(CurrentPlatform.ComponentsIncludePath);
		PublicIncludePaths.Add(CurrentPlatform.DependenciesIncludePath);
		PublicIncludePaths.Add(CurrentPlatform.TerrainDependenciesIncludePath);
		PublicIncludePaths.Add(CurrentPlatform.ConfigIncludePath);
	}

	private void PackageAGXDynamicsIntoPlugin()
	{
		string PluginBinariesDirectory = Path.GetFullPath(Path.Combine(new string[] {
			PluginDirectory, "Binaries", Target.Platform.ToString()}));
		if (!Directory.Exists(PluginBinariesDirectory))
		{
			Directory.CreateDirectory(PluginBinariesDirectory);
		}

		// TODO: Why do I need to do the copy the .dll/.so if I have already
		//       added the library to RuntimeDependencies?
		string Source = CurrentPlatform.RuntimeLibraryPath("agxCore");
		string Destination = Path.Combine(PluginBinariesDirectory, CurrentPlatform.RuntimeLibraryFileName("agxCore"));
		if (!File.Exists(Destination))
		{
			File.Copy(
				Source,
				Destination,
				overwrite: true);
		}

	}

	private class PlatformInfo
	{
		public string LinkLibraryPostfix;
		public string LinkLibraryPrefix;
		public string RuntimeLibraryPrefix;
		public string RuntimeLibraryPostfix;

		public string LinkLibrariesDirectory;
		public string RuntimeLibrariesDirectory;
		public string TerrainDependenciesLinkLibrariesDirectory;
		public string TerrainDependenciesRuntimeLibrariesDirectory;
		// TODO: May need dependencies here as well.

		// TODO: Consider making these an array instead.
		public string LibraryIncludePath;
		public string ComponentsIncludePath;
		public string DependenciesIncludePath;
		public string TerrainDependenciesIncludePath;
		public string ConfigIncludePath;

		public string LinkLibraryFileName(string LibraryName)
		{
			return LinkLibraryPrefix + LibraryName + LinkLibraryPostfix;
		}

		public string RuntimeLibraryFileName(string LibraryName)
		{
			return RuntimeLibraryPrefix + LibraryName + RuntimeLibraryPostfix;
		}

		public string LinkLibraryPath(string LibraryName)
		{
			return Path.Combine(LinkLibrariesDirectory, LinkLibraryFileName(LibraryName));
		}

		public string LinkTerrainDependencyLibraryPath(string LibraryName)
		{
		    return Path.Combine(TerrainDependenciesLinkLibrariesDirectory, LinkLibraryFileName(LibraryName));
		}

		public string RuntimeLibraryPath(string LibraryName)
		{
			return Path.Combine(RuntimeLibrariesDirectory, RuntimeLibraryFileName(LibraryName));
		}

		public string RuntimeTerrainDependencyLibraryPath(string LibraryName)
		{
		    return Path.Combine(TerrainDependenciesRuntimeLibrariesDirectory);
		}

		public PlatformInfo(ReadOnlyTargetRules Target)
		{
			// TODO: Detect if AGD Dynamics is in local build or installed mode.
			//       Currently assuming local build for Linux and installed for Windows.
			string AGXDir = Environment.GetEnvironmentVariable("AGX_DIR");
			string AGXBuildDir = Environment.GetEnvironmentVariable("AGX_BUILD_DIR") ?? AGXDir;
			string AGXDependenciesDir = Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR") ?? AGXDir;
			string AGXTerrainDependenciesDir = Environment.GetEnvironmentVariable("AGXTERRAIN_DEPENDENCIES_DIR") ?? AGXDir;
			if (AGXDir == null || AGXBuildDir == null || AGXDependenciesDir == null || AGXTerrainDependenciesDir == null)
			{
				System.Console.WriteLine("Did not find AGX Dynamics installation folder.");
				System.Console.WriteLine("Have you run setup_env?");
				return;
			}

			if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				LinkLibraryPrefix = "lib";
				LinkLibraryPostfix = ".so";
				RuntimeLibraryPrefix = "lib";
				RuntimeLibraryPostfix = ".so";
				LinkLibrariesDirectory = Path.Combine(AGXBuildDir, "lib");
				RuntimeLibrariesDirectory = Path.Combine(AGXBuildDir, "lib");
				LibraryIncludePath = Path.Combine(AGXDir, "include");
				ComponentsIncludePath = Path.Combine(AGXDir, "Components");
				DependenciesIncludePath = Path.Combine(AGXDependenciesDir, "include");
				TerrainDependenciesIncludePath = Path.Combine(AGXTerrainDependenciesDir, "include");
				TerrainDependenciesLinkLibrariesDirectory = Path.Combine(AGXTerrainDependenciesDir, "lib");
				TerrainDependenciesRuntimeLibrariesDirectory = Path.Combine(AGXTerrainDependenciesDir, "lib");
				ConfigIncludePath = Path.Combine(AGXBuildDir, "include");
			}
			else if(Target.Platform == UnrealTargetPlatform.Win64)
			{
				LinkLibraryPrefix = "";
				LinkLibraryPostfix = ".lib";
				RuntimeLibraryPrefix = "";
				RuntimeLibraryPostfix = ".dll";
				LinkLibrariesDirectory = Path.Combine(AGXDir, "lib", "x64");
				RuntimeLibrariesDirectory = Path.Combine(AGXDir, "bin", "x64");
				LibraryIncludePath = Path.Combine(AGXDir, "include");
				ComponentsIncludePath = Path.Combine(AGXDir, "include");
				DependenciesIncludePath = Path.Combine(AGXDir, "include");
				TerrainDependenciesIncludePath = Path.Combine(AGXDir, "include");
				TerrainDependenciesLinkLibrariesDirectory = Path.Combine(AGXDir, "lib", "x64");
				TerrainDependenciesRuntimeLibrariesDirectory = Path.Combine(AGXDir, "bin", "x64");
				ConfigIncludePath = Path.Combine(AGXDir, "include");
			}

			if (Target.Configuration == UnrealTargetConfiguration.Debug)
			{
				// Always building against the release AGX Dynamics on Linux for
				// now. Only because it is difficult to switch between release
				// and debug on Linux.
				if (Target.Platform != UnrealTargetPlatform.Linux) {
					string DebugSuffix = "d";
					LinkLibraryPostfix = DebugSuffix + LinkLibraryPostfix;
					RuntimeLibraryPostfix = DebugSuffix + RuntimeLibraryPostfix;
				}
			}
		}
	}
}
