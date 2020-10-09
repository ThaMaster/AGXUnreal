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
		TerrainDependencies,
		Cfg
	};

	// This build script will be run mainly in two situations:
	// 1. When building/packaging the AGXUnreal plugin itself.
	// 2. When building an executable from a project that uses the AGX Dynamics for Unreal plugin.
	// In both cases all necessary AGX Dynamics resources are packaged with the target
	// so that it is then possible to use plugin or executable without the need to call AGX's
	// setup_env. Also, the needed AGX Dynamics link libraries are packaged with the target so
	// that it becomes possible to build an executable from a project using the AGXUnreal plugin,
	// even if setup_env has not been called.
	//
	// Details situation 1 (building/packaging the plugin):
	// AGX Dynamics binary files (.dll/.so), link library files and header files must be
	// available at build time. AGX's setup_env must always be set up when building/packaging
	// the plugin itself. All AGX Dynamics runtime resources are copied from the AGX Dynamics
	// installation directory and are packaged with the target.
	// Also note: AGX Dynamics header files are never packaged with the plugin, these will only
	// be available if an AGX environment has been set up (setup_env has been called).
	//
	// Details situation 2 (building an executable):
	// AGX Dynamics binary files (.dll/.so) and link library files must be available at build time
	// (not headers).
	// If AGX's setup_env has been called, all AGX Dynamics resources will be taken from the AGX
	// Dynamics installation directory.
	// It is also possible to build an executable without calling AGX's setup_env if and only if the
	// necessary AGX Dynamics resources has been packaged with the AGXUnreal plugin itself.

	public AGXUnrealLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		CurrentPlatform = new PlatformInfo(Target, PluginDirectory);
		Type = ModuleType.External;

		// Because the AGX Dynamics type system uses typeid and dynamic_cast.
		bUseRTTI = true;

		// Because AGX Dynamics uses exceptions.
		bEnableExceptions = true;

		// '*' Means to include all files in the directory.
		AddRuntimeDependencyDirectory("*", LibSource.Components, "agx/Physics",
			"agx/plugins/Components/agx/Physics");
		AddRuntimeDependencyDirectory("*", LibSource.Cfg, string.Empty, "agx/data/cfg");

		AddDependencyExplicitFile("Referenced.agxEntity",
			CurrentPlatform.RuntimeLibraryDirectory(LibSource.Components),
			"agx", "agx/plugins/Components/agx");

		AddRuntimeDependency("agxPhysics", LibSource.Agx);
		AddRuntimeDependency("agxCore", LibSource.Agx);
		AddRuntimeDependency("agxSabre", LibSource.Agx);
		AddRuntimeDependency("agxTerrain", LibSource.Agx);
		AddRuntimeDependency("agxCable", LibSource.Agx);
		AddRuntimeDependency("agxModel", LibSource.Agx);
		AddRuntimeDependency("vdbgrid", LibSource.Agx);
		AddRuntimeDependency("colamd", LibSource.Agx);

		AddRuntimeDependency("zlib", LibSource.Dependencies);

		AddRuntimeDependency("Half", LibSource.TerrainDependencies);
		AddRuntimeDependency("openvdb", LibSource.TerrainDependencies);

		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			AddRuntimeDependency("png", LibSource.Dependencies);

			// TODO: Use Unreal Engine packaged TBB.
			//
			// tbb, i.e., Intel Threading Building Blocks, is problematic
			// because it is also included in Unreal Engine itself. We should
			// build AGX Dynamics' dependencies against those binaries of tbb
			// but our dependencies build pipeline currently doesn't support
			// that. Including this line on Windows causes build errors when
			// building an executable from a project using this plugin. On Linux
			// this line must be here because otherwise we get linker errors at
			// startup.
			AddRuntimeDependency("tbb", LibSource.TerrainDependencies);

			// We must list OpenVDB only because of problems with
			// initialization. We should try to figure out what goes wrong.
			AddLinkLibrary("openvdb", LibSource.TerrainDependencies);
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			AddRuntimeDependency("msvcp140", LibSource.Agx);
			AddRuntimeDependency("vcruntime140", LibSource.Agx);

			AddRuntimeDependency("websockets", LibSource.Dependencies);
			AddRuntimeDependency("libpng", LibSource.Dependencies);
			AddRuntimeDependency("ot2*-OpenThreads", LibSource.Dependencies);
			AddRuntimeDependency("glew", LibSource.Dependencies);
		}

		AddLinkLibrary("agxPhysics", LibSource.Agx);
		AddLinkLibrary("agxCore", LibSource.Agx);
		AddLinkLibrary("agxSabre", LibSource.Agx);
		AddLinkLibrary("agxTerrain", LibSource.Agx);
		AddLinkLibrary("agxCable", LibSource.Agx);
		AddLinkLibrary("agxModel", LibSource.Agx);

		// @todo IncludePaths are only needed when building/packaging
		// the plugin itself (not when building an executable). So the below if-statement
		// would optimally check if we are building/packaging the plugin instead.
		// Currently no reliable way of telling the two situations apart here has been found.
		// Therefore we simply use the rule that if an AGX environment has been detected,
		// they are always added. When building/packaging the plugin itself an AGX
		// environment MUST be set up so for that situation the logic is correct.
		// For the case that an executable is being built and an AGX environment is
		// set up, adding these will not affect the build result.
		if (CurrentPlatform.UseInstalledAgx)
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

	// The runtime dependency directory will be automatically copied to the target.
	private void AddRuntimeDependencyDirectory(string Name, LibSource Src, string SourceAppendPath = "",
		string TargetAppendPath = "")
	{
		string Source = CurrentPlatform.RuntimeLibraryPath(Name, Src, SourceAppendPath, true);
		string Target = Path.Combine("$(BinaryOutputDir)", TargetAppendPath);

		RuntimeDependencies.Add(Target, Source);
	}

	// The dependency file will be automatically copied to the target.
	private void AddDependencyExplicitFile(string Name, string SourceDirectory, string SourceAppendPath = "",
		string TargetAppendPath = "")
	{
		string Source = Path.Combine(SourceDirectory, SourceAppendPath, Name);
		string Target = Path.Combine("$(BinaryOutputDir)", TargetAppendPath, Name);

		RuntimeDependencies.Add(Target, Source);
	}

	// The runtime dependency file will be automatically copied to the target.
	private void AddRuntimeDependency(string Name, LibSource Src, string SourceAppendPath = "",
		string TargetAppendPath = "")
	{
		List<string> FilesToAdd = new List<string>();

		if (Name.Contains("*"))
		{
			// Find all files matching the given pattern.
			string Directory = Path.Combine(CurrentPlatform.RuntimeLibraryDirectory(Src), SourceAppendPath);
			FilesToAdd = FindMatchingFiles(Directory, Name);
		}
		else
		{
			FilesToAdd.Add(Name);
		}

		if (FilesToAdd.Count == 0)
		{
			Console.WriteLine("File {0} did not match any found files on disk. " +
				"The dependency will not be added in the build.", Name);
		}

		foreach (string FileName in FilesToAdd)
		{
			string FileNameFull = CurrentPlatform.RuntimeLibraryFileName(FileName);
			string Target = Path.Combine("$(BinaryOutputDir)", TargetAppendPath, FileNameFull);
			string Source = CurrentPlatform.RuntimeLibraryPath(FileName, Src, SourceAppendPath);
			RuntimeDependencies.Add(Target, Source);
		}
	}

	private void AddLinkLibrary(string Name, LibSource Src)
	{
		List<string> FilesToAdd = new List<string>();

		if (Name.Contains("*"))
		{
			// Find all files matching the given pattern.
			FilesToAdd = FindMatchingFiles(CurrentPlatform.LinkLibraryDirectory(Src), Name);
		}
		else
		{
			FilesToAdd.Add(Name);
		}

		if (FilesToAdd.Count == 0)
		{
			Console.WriteLine("File {0} did not match any found files on disk. The library will not be added " +
				"in the build.", Name);
		}

		foreach (string FileName in FilesToAdd)
		{
			PublicAdditionalLibraries.Add(CurrentPlatform.LinkLibraryPath(FileName, Src));

			// Copy the link library file to the target.
			// @todo Copying the lib files are only necessary when building/packaging the plugin
			// itself, not when building an executable. Currently no reliable way of separating
			// the two here have been found, so right now the link library files will be copied
			// to the target when building executables also even if they are not needed in that
			// case.
			string FileNameFull = CurrentPlatform.LinkLibraryFileName(FileName);
			string TargetRelativePath = "../../lib";

			if(Target.Platform == UnrealTargetPlatform.Win64)
			{
				TargetRelativePath = Path.Combine(TargetRelativePath, "Win64");
			}

			AddDependencyExplicitFile(FileNameFull, CurrentPlatform.LinkLibraryDirectory(Src), "",
				TargetRelativePath);
		}
	}

	private List<string> FindMatchingFiles(string Dir, string FileName)
	{
		string[] Matches = Directory.GetFiles(Dir, FileName + ".*");
		List<string> Res = new List<string>();
		foreach (string Match in Matches)
		{
			Res.Add(Path.GetFileNameWithoutExtension(Match));
		}

		return Res;
	}

	private class Heuristics
	{
		/**
		From the plugin's point of view AGX Dynamics can be in one of two states: Installed or Packaged.
		Installed means that AGX Dynamics exists somewhere outside of the plugin. It does not mean that
		it must be an actual installation, a source build also counts as an installation. A source build
		is identified by the presence of the AGX Dynamics environment variable AGX_DEPENDENCIES_DIR, which is set
		when AGX Dynamics' setup_env is run. If the AGX_DEPENDENCIES_DIR environment variable isn't set then it is
		assumed that AGX Dynamics is packaged with the plugin and all AGX Dynamics search path will be
		relative to the plugin directory.
		*/
		public static bool UseInstalledAgx(UnrealTargetPlatform Platform)
		{
			bool bHasInstalledAgx = Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR") != null;
			if (bHasInstalledAgx)
			{
				Console.WriteLine(
					"\nUsing AGX Dynamics installation {0}.",
					Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR"));
			}
			else
			{
				Console.WriteLine(
					"\nNo installation of AGX Dynamics detected, using version packaged with the plugin.");
			}
			return bHasInstalledAgx;
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

		public bool UseInstalledAgx;

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
				Console.Error.WriteLine("No LinkLibraryPath for '{0}', '{1}' cannot be found.", Src, LibraryName);
				return LibraryName;
			}
			return Path.Combine(Info.LinkLibrariesPath, LinkLibraryFileName(LibraryName));
		}

		public string LinkLibraryDirectory(LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.LinkLibrariesPath == null)
			{
				Console.Error.WriteLine("No LinkLibraryPath for '{0}'.", Src);
				return string.Empty;
			}
			return Info.LinkLibrariesPath;
		}

		public string RuntimeLibraryPath(string LibraryName, LibSource Src, string AppendPath = "",
			bool IsDirectory = false)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.RuntimeLibrariesPath == null)
			{
				Console.Error.WriteLine("No RuntimeLibraryPath for '{0}', '{1}' cannot be found.", Src,
					LibraryName);
				return LibraryName;
			}
			if (IsDirectory)
			{
				return Path.Combine(Info.RuntimeLibrariesPath, AppendPath, LibraryName);
			}
			else
			{
				return Path.Combine(Info.RuntimeLibrariesPath, AppendPath, RuntimeLibraryFileName(LibraryName));
			}
		}

		public string RuntimeLibraryDirectory(LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.RuntimeLibrariesPath == null)
			{
				Console.Error.WriteLine("No RuntimeLibraryDirectory for '{0}'.", Src);
				return string.Empty;
			}
			return Info.RuntimeLibrariesPath;
		}

		private string GetComponentsRuntimePath(ReadOnlyTargetRules Target, string BaseDir,
			bool UseInstalledAgx)
		{
			if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				if (UseInstalledAgx)
				{
					// On Linux, depending if this is a built or installed Agx Dynamics, the environment
					// variable AGX_PLUGIN_PATH may return several paths which may or may not include the
					// sought after Components directory. If none does, it is assumed that the Components
					// directory is located directly in one of the plugin paths.
					string[] PluginPaths = Environment.GetEnvironmentVariable("AGX_PLUGIN_PATH").Split(':');

					foreach (string PluginPath in PluginPaths)
					{
						string ParentCandidate = PluginPath;
						if (ParentCandidate.Contains("Components"))
						{
							ParentCandidate = ParentCandidate.Substring(0, ParentCandidate.LastIndexOf("Components"));
						}

						bool IsValidParent = Directory.Exists(Path.Combine(ParentCandidate, "Components", "agx"));
						if (IsValidParent)
						{
							return Path.Combine(ParentCandidate, "Components");
						}
					}

					Console.WriteLine("Unable to find installed Agx Dynamics Components runtime path");
					return string.Empty;
				}
				else
				{
					return Path.Combine(BaseDir, "Binaries", "Linux", "agx", "plugins", "Components");
				}

			}
			else if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				if (UseInstalledAgx)
				{
					return Path.Combine(Environment.GetEnvironmentVariable("AGX_PLUGIN_PATH"), "Components");
				}
				else
				{
					return Path.Combine(BaseDir, "Binaries", "Win64", "agx", "plugins", "Components");
				}
			}
			else
			{
				Console.WriteLine("Could not get Components runtime path from unsupported platform: " +
					Target.Platform);
				return string.Empty;
			}
		}

		public PlatformInfo(ReadOnlyTargetRules Target, string PluginDir)
		{
			LibSources = new Dictionary<LibSource, LibSourceInfo>();

			this.UseInstalledAgx = Heuristics.UseInstalledAgx(Target.Platform);

			// TODO: Detect if AGX Dynamics is in local build or installed mode.
			//	   Currently assuming local build for Linux and installed for Windows.
			string BaseDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_DIR") : PluginDir;
			string BuildDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_BUILD_DIR")
				?? BaseDir : PluginDir;
			string DependenciesDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR")
				?? BaseDir : PluginDir;
			string TerrainDependenciesDir = UseInstalledAgx ?
				Environment.GetEnvironmentVariable("AGXTERRAIN_DEPENDENCIES_DIR")
				?? BaseDir : PluginDir;
			if (UseInstalledAgx && (BaseDir == null || BuildDir == null || DependenciesDir == null
				|| TerrainDependenciesDir == null))
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
				RuntimeLibraryPostfix = ".so*";

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
					GetComponentsRuntimePath(Target, BaseDir, UseInstalledAgx)
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

				// On Linux, environment variable 'AGX_DATA_DIR' is not always visible here when using
				//  abuilt Agx Dynamics. Therefore 'AGX_DIR' is used instead to find the cfg directory.
				LibSources.Add(LibSource.Cfg, new LibSourceInfo(
					null,
					null,
					UseInstalledAgx ?
						Path.Combine(Environment.GetEnvironmentVariable("AGX_DIR"), "data", "cfg")
						: Path.Combine(BaseDir, "Binaries", "Linux", "agx", "data", "cfg")
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
					UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64")
						: Path.Combine(BaseDir, "lib", "Win64"),
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64")
						: Path.Combine(BaseDir, "Binaries", "Win64")
				));

				LibSources.Add(LibSource.Config, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					null,
					null
				));

				LibSources.Add(LibSource.Components, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					null,
					GetComponentsRuntimePath(Target, BaseDir, UseInstalledAgx)
				));

				LibSources.Add(LibSource.Dependencies, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64"),
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "Binaries", "Win64")
				));

				LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64"),
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "Binaries", "Win64")
				));

				LibSources.Add(LibSource.Cfg, new LibSourceInfo(
					null,
					null,
					UseInstalledAgx ? Path.Combine(Environment.GetEnvironmentVariable("AGX_DATA_DIR"), "cfg")
						: Path.Combine(BaseDir, "Binaries", "Win64", "agx", "data", "cfg")
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
