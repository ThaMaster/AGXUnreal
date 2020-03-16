using System.IO;
using System;
using System.Collections.Generic;
using UnrealBuildTool;


/// The AGXUnrealLibrary is the portal from AGXUnrealBarrier to AGX Dynamics.
/// This is where we list the include paths and linker requirements to build an
/// Unreal Engine plugin that uses AGX Dynamics.
public class AGXUnrealLibrary : ModuleRules
{
	/// Information about how AGX Dynamics is packaged and used on the current
	/// platform.
	private PlatformInfo CurrentPlatform;

	// The various dependency sources we have. Each come with an include path,
	// a linker path and a runtime path. The PlatformInfo is responsible for
	// keeping the LibSource->Paths associations.
	private enum LibSource {
		Agx,
		Config,
		Components,
		Dependencies,
		TerrainDependencies
	};

	public AGXUnrealLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		CurrentPlatform = new PlatformInfo(Target, PluginDirectory);
		Type = ModuleType.External;

		// Because the AGX Dynamics type system uses typeid and dynamic_cast.
		bUseRTTI = true;

		// Because AGX Dynamics uses exceptions.
		bEnableExceptions = true;

		bool ManualCopy = (Target.Type == TargetType.Editor && bTreatAsEngineModule);

		AddLinkLibrary("agxPhysics", LibSource.Agx, ManualCopy);
		AddLinkLibrary("agxCore", LibSource.Agx, ManualCopy);
		AddLinkLibrary("agxSabre", LibSource.Agx, ManualCopy);
		AddLinkLibrary("agxTerrain", LibSource.Agx, ManualCopy);

		AddRuntimeDependency("agxPhysics", LibSource.Agx, ManualCopy);
		AddRuntimeDependency("agxCore", LibSource.Agx, ManualCopy);
		AddRuntimeDependency("agxSabre", LibSource.Agx, ManualCopy);
		AddRuntimeDependency("agxTerrain", LibSource.Agx, ManualCopy);
		AddRuntimeDependency("vdbgrid", LibSource.Agx, ManualCopy);

		AddRuntimeDependency("colamd", LibSource.Dependencies, ManualCopy);
		AddRuntimeDependency("glew", LibSource.Dependencies, ManualCopy);
		AddRuntimeDependency("libpng", LibSource.Dependencies, ManualCopy);
		AddRuntimeDependency("zlib", LibSource.Dependencies, ManualCopy);

		AddRuntimeDependency("Half", LibSource.TerrainDependencies, ManualCopy);
		AddRuntimeDependency("openvdb", LibSource.TerrainDependencies, ManualCopy);
		AddRuntimeDependency("tbb", LibSource.TerrainDependencies, ManualCopy);
		AddRuntimeDependency("websockets", LibSource.Dependencies, ManualCopy);

		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			AddRuntimeDependency("OpenThreads", LibSource.Dependencies, ManualCopy);

			// OpenVDB is only required because of problems with initialization.
			// We should try to figure out what goes wrong.
			AddLinkLibrary("openvdb", LibSource.TerrainDependencies, ManualCopy);
		}
		else if(Target.Platform == UnrealTargetPlatform.Win64)
		{
			AddRuntimeDependency("ot20-OpenThreads", LibSource.Dependencies, ManualCopy);
			// What are these? And more importantly, where are they?
			Console.Error.WriteLine("I don't know where msvcp140 and vcruntime140 are.");
			// AddRuntimeDependency("msvcp140", ManualCopy);
			// AddRuntimeDependency("vcruntime140", ManualCopy);
		}

		if(Target.Type == TargetType.Editor)
		{
			AddIncludePath(LibSource.Agx);
			AddIncludePath(LibSource.Components);
			AddIncludePath(LibSource.Config);
			AddIncludePath(LibSource.Dependencies);
			AddIncludePath(LibSource.TerrainDependencies);
		}
	}

	private void AddIncludePath(LibSource Src)
	{
		PublicIncludePaths.Add(CurrentPlatform.IncludePath(Src));
	}

	private void AddRuntimeDependency(string Name, LibSource Src, bool PerformManualCopy = false)
	{
		RuntimeDependencies.Add(CurrentPlatform.RuntimeLibraryPath(Name, Src));

		if(PerformManualCopy)
		{
			string PluginRuntimeBinariesDirectory = Path.GetFullPath(Path.Combine(new string[] {
				Directory.GetParent(PluginDirectory).Parent.Parent.FullName, "Binaries", Target.Platform.ToString()}));

			if (!Directory.Exists(PluginRuntimeBinariesDirectory))
			{
				Directory.CreateDirectory(PluginRuntimeBinariesDirectory);
			}

			// TODO: Why do I need to do the copy the .dll/.so if I have already
			// added the library to RuntimeDependencies?
			string Source = CurrentPlatform.RuntimeLibraryPath(Name, Src);
			string Destination = Path.Combine(PluginRuntimeBinariesDirectory, CurrentPlatform.RuntimeLibraryFileName(Name));

			File.Copy(
				Source,
				Destination,
				overwrite: true);
		}
	}

	private void AddLinkLibrary(string Name, LibSource Src, bool PerformManualCopy = false)
	{
		PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath(Name, Src));

		if (PerformManualCopy)
		{
			string PluginLinkLibraryDirectory = Path.GetFullPath(Path.Combine(new string[] {
				Directory.GetParent(PluginDirectory).Parent.Parent.FullName, "lib", Target.Platform.ToString()}));

			if (!Directory.Exists(PluginLinkLibraryDirectory))
			{
				Directory.CreateDirectory(PluginLinkLibraryDirectory);
			}

			// TODO: Why do I need to do the copy the .dll/.so if I have already
			//	   added the library to RuntimeDependencies?
			string Source = CurrentPlatform.LinkLibraryPath(Name, Src);
			string Destination = Path.Combine(PluginLinkLibraryDirectory, CurrentPlatform.LinkLibraryFileName(Name));

			File.Copy(
				Source,
				Destination,
				overwrite: true);
		}
	}

	private class LibSourceInfo
	{
		public string IncludePath;
		public string LinkLibrariesPath;
		public string RuntimeLibrariesPath;

		public LibSourceInfo(string InIncludePath, string InLinkLibrariesPath, string InRuntimeLibrariesPath)
		{
			IncludePath = InIncludePath;
			LinkLibrariesPath = InLinkLibrariesPath;
			RuntimeLibrariesPath = InRuntimeLibrariesPath;
		}
	}

	private class PlatformInfo
	{
		public string LinkLibraryPrefix;
		public string LinkLibraryPostfix;

		public string RuntimeLibraryPrefix;
		public string RuntimeLibraryPostfix;

		Dictionary<LibSource, LibSourceInfo> LibSources;

		public string LinkLibraryFileName(string LibraryName)
		{
			return LinkLibraryPrefix + LibraryName + LinkLibraryPostfix;
		}

		public string RuntimeLibraryFileName(string LibraryName)
		{
			return RuntimeLibraryPrefix + LibraryName + RuntimeLibraryPostfix;
		}

		public string IncludePath(LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.IncludePath == null)
			{
				Console.Error.WriteLine("No include path for '{0}'.", Src);
				return null;
			}
			return Info.IncludePath;
		}

		public string LinkLibraryPath(string LibraryName, LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.LinkLibrariesPath == null)
			{
				Console.Error.WriteLine("NoLinkLibraryPath for '{0}', '{1}' cannot be found.", Src, LibraryName);
				return LibraryName;
			}
			return Path.Combine(Info.LinkLibrariesPath, LinkLibraryFileName(LibraryName));
		}

		public string RuntimeLibraryPath(string LibraryName, LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.RuntimeLibrariesPath == null)
			{
				Console.Error.WriteLine("No RuntimeLibraryPath for '{0}', '{1}' cannot be found.", Src, LibraryName);
				return LibraryName;
			}
			return Path.Combine(Info.RuntimeLibrariesPath, RuntimeLibraryFileName(LibraryName));
		}

		public PlatformInfo(ReadOnlyTargetRules Target, string PluginDir)
		{
			LibSources = new Dictionary<LibSource, LibSourceInfo>();

			bool UseInstalledAgx = Target.Type != TargetType.Game;

			// TODO: Detect if AGX Dynamics is in local build or installed mode.
			//	   Currently assuming local build for Linux and installed for Windows.
			string BaseDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_DIR") : PluginDir;
			string BuildDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_BUILD_DIR") ?? BaseDir : PluginDir;
			string DependenciesDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR") ?? BaseDir : PluginDir;
			string TerrainDependenciesDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGXTERRAIN_DEPENDENCIES_DIR") ?? BaseDir : PluginDir;
			if (UseInstalledAgx && (BaseDir == null || BuildDir == null || DependenciesDir == null || TerrainDependenciesDir == null))
			{
				Console.Error.WriteLine("Did not find AGX Dynamics installation folder.");
				Console.Error.WriteLine("Have you run setup_env?");
				return;
			}

			if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				LinkLibraryPrefix = "lib";
				LinkLibraryPostfix = ".so";
				RuntimeLibraryPrefix = "lib";
				RuntimeLibraryPostfix = ".so";

				LibSources.Add(LibSource.Agx, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					Path.Combine(BuildDir, "lib"),
					Path.Combine(BuildDir, "lib")
				));

				LibSources.Add(LibSource.Config, new LibSourceInfo(
					Path.Combine(BuildDir, "include"),
					null,
					null
				));

				LibSources.Add(LibSource.Components, new LibSourceInfo(
					Path.Combine(BaseDir, "Components"),
					null,
					null
				));

				LibSources.Add(LibSource.Dependencies, new LibSourceInfo(
					Path.Combine(DependenciesDir, "include"),
					Path.Combine(DependenciesDir, "lib"),
					Path.Combine(DependenciesDir, "lib")
				));

				LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
					Path.Combine(TerrainDependenciesDir, "include"),
					Path.Combine(TerrainDependenciesDir, "lib"),
					Path.Combine(TerrainDependenciesDir, "lib")
				));
			}
			else if(Target.Platform == UnrealTargetPlatform.Win64)
			{
				LinkLibraryPrefix = "";
				LinkLibraryPostfix = ".lib";
				RuntimeLibraryPrefix = "";
				RuntimeLibraryPostfix = ".dll";

				LibSources.Add(LibSource.Agx, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64"),
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "Binaries", "Win64")
				));

				LibSources.Add(LibSource.Config, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					null,
					null
				));

				LibSources.Add(LibSource.Components, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					null,
					null
				));

				LibSources.Add(LibSource.Dependencies, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64"),
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "Binaries", "Win64")
				));

				LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64"),
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "lib", "Win64")
				));
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
