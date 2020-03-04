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
		CurrentPlatform = new PlatformInfo(Target, PluginDirectory);
		Type = ModuleType.External;

		// Because the AGX Dynamics type system uses typeid and dynamic_cast.
		bUseRTTI = true;

		// Because AGX Dynamics uses exceptions.
		bEnableExceptions = true;

		bool ManualCopy = (Target.Type == TargetType.Editor && bTreatAsEngineModule);

		AddLinkLibrary("agxPhysics", ManualCopy);
		AddLinkLibrary("agxCore", ManualCopy);
		AddLinkLibrary("agxSabre", ManualCopy);
		AddLinkLibrary("agxTerrain", ManualCopy);

		AddRuntimeDependency("agxPhysics", ManualCopy);
		AddRuntimeDependency("agxCore", ManualCopy);
		AddRuntimeDependency("agxSabre", ManualCopy);
		AddRuntimeDependency("agxTerrain", ManualCopy);

		AddRuntimeDependency("colamd", ManualCopy);
		AddRuntimeDependency("glew", ManualCopy);
		AddRuntimeDependency("Half", ManualCopy);
		AddRuntimeDependency("libpng", ManualCopy);
		AddRuntimeDependency("msvcp140", ManualCopy);
		AddRuntimeDependency("openvdb", ManualCopy);
		AddRuntimeDependency("ot20-OpenThreads", ManualCopy);
		AddRuntimeDependency("tbb", ManualCopy);
		AddRuntimeDependency("vcruntime140", ManualCopy);
		AddRuntimeDependency("vdbgrid", ManualCopy);
		AddRuntimeDependency("websockets", ManualCopy);
		AddRuntimeDependency("zlib", ManualCopy);
	}

	private void AddRuntimeDependency(string Name, bool PerformManualCopy = false)
	{
		RuntimeDependencies.Add(CurrentPlatform.RuntimeLibraryPath(Name));

		if(PerformManualCopy)
		{
			string PluginRuntimeBinariesDirectory = Path.GetFullPath(Path.Combine(new string[] {
				Directory.GetParent(PluginDirectory).Parent.Parent.FullName, "Binaries", Target.Platform.ToString()}));

			if (!Directory.Exists(PluginRuntimeBinariesDirectory))
			{
				Directory.CreateDirectory(PluginRuntimeBinariesDirectory);
			}

			// TODO: Why do I need to do the copy the .dll/.so if I have already
			//       added the library to RuntimeDependencies?
			string Source = CurrentPlatform.RuntimeLibraryPath(Name);
			string Destination = Path.Combine(PluginRuntimeBinariesDirectory, CurrentPlatform.RuntimeLibraryFileName(Name));

			File.Copy(
				Source,
				Destination,
				overwrite: true);
		}
	}

	private void AddLinkLibrary(string Name, bool PerformManualCopy = false)
	{
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath(Name));

		if (PerformManualCopy)
		{
			string PluginLinkLibraryDirectory = Path.GetFullPath(Path.Combine(new string[] {
				Directory.GetParent(PluginDirectory).Parent.Parent.FullName, "lib", Target.Platform.ToString()}));

			if (!Directory.Exists(PluginLinkLibraryDirectory))
			{
				Directory.CreateDirectory(PluginLinkLibraryDirectory);
			}

			// TODO: Why do I need to do the copy the .dll/.so if I have already
			//       added the library to RuntimeDependencies?
			string Source = CurrentPlatform.LinkLibraryPath(Name);
			string Destination = Path.Combine(PluginLinkLibraryDirectory, CurrentPlatform.LinkLibraryFileName(Name));

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

		public PlatformInfo(ReadOnlyTargetRules Target, string PluginDir)
		{
			bool UseInstalledAgx = Target.Type != TargetType.Game;

			// TODO: Detect if AGD Dynamics is in local build or installed mode.
			//       Currently assuming local build for Linux and installed for Windows.
			string BaseDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_DIR") : PluginDir;
			string BuildDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_BUILD_DIR") ?? BaseDir : PluginDir;
			string DependenciesDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR") ?? BaseDir : PluginDir;
			string TerrainDependenciesDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGXTERRAIN_DEPENDENCIES_DIR") ?? BaseDir : PluginDir;
			if (UseInstalledAgx && (BaseDir == null || BuildDir == null || DependenciesDir == null || TerrainDependenciesDir == null))
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
				LinkLibrariesDirectory = Path.Combine(BuildDir, "lib");
				RuntimeLibrariesDirectory = Path.Combine(BuildDir, "lib");
				LibraryIncludePath = Path.Combine(BaseDir, "include");
				ComponentsIncludePath = Path.Combine(BaseDir, "Components");
				DependenciesIncludePath = Path.Combine(DependenciesDir, "include");
				TerrainDependenciesIncludePath = Path.Combine(TerrainDependenciesDir, "include");
				TerrainDependenciesLinkLibrariesDirectory = Path.Combine(TerrainDependenciesDir, "lib");
				TerrainDependenciesRuntimeLibrariesDirectory = Path.Combine(TerrainDependenciesDir, "lib");
				ConfigIncludePath = Path.Combine(BuildDir, "include");
			}
			else if(Target.Platform == UnrealTargetPlatform.Win64)
			{
				LinkLibraryPrefix = "";
				LinkLibraryPostfix = ".lib";
				RuntimeLibraryPrefix = "";
				RuntimeLibraryPostfix = ".dll";
				LinkLibrariesDirectory = UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64");
				RuntimeLibrariesDirectory = UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "Binaries", "Win64");
				LibraryIncludePath = Path.Combine(BaseDir, "include");
				ComponentsIncludePath = Path.Combine(BaseDir, "include");
				DependenciesIncludePath = Path.Combine(BaseDir, "include");
				TerrainDependenciesIncludePath = Path.Combine(BaseDir, "include");
				TerrainDependenciesLinkLibrariesDirectory = UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64");
				TerrainDependenciesRuntimeLibrariesDirectory = UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "lib", "Win64");
				ConfigIncludePath = Path.Combine(BaseDir, "include");
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
