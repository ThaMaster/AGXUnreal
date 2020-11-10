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
	private AgxResourcesInfo PackagedAgxResources;

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

	private enum AgxResourcesLocation {
		InstalledAgx,
		PackagedAgx
	}

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
		PackagedAgxResources =
			new AgxResourcesInfo(Target, AgxResourcesLocation.PackagedAgx, GetPackagedAgxResourcesPath());
		Type = ModuleType.External;

		// Because the AGX Dynamics type system uses typeid and dynamic_cast.
		bUseRTTI = true;

		// Because AGX Dynamics uses exceptions.
		bEnableExceptions = true;

		if (!IsAgxResourcesPackaged())
		{
			PackageAgxResources();
		}

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


		AddIncludePath(LibSource.Agx);
		AddIncludePath(LibSource.Components);
		AddIncludePath(LibSource.Config);
		AddIncludePath(LibSource.Dependencies);
		AddIncludePath(LibSource.TerrainDependencies);
	}

	private void AddIncludePath(LibSource Src)
	{
		PublicIncludePaths.Add(PackagedAgxResources.IncludePath(Src));
	}

	// The runtime dependency file is copied to the target binaries directory.
	private void AddRuntimeDependency(string Name, LibSource Src)
	{
		List<string> FilesToAdd = new List<string>();

		if (Name.Contains("*"))
		{
			// Find all files matching the given pattern.
			string Directory = Path.Combine(PackagedAgxResources.RuntimeLibraryDirectory(Src));
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
			string FileNameFull = PackagedAgxResources.RuntimeLibraryFileName(FileName);
			string Target = Path.Combine("$(BinaryOutputDir)", FileNameFull);
			string Source = PackagedAgxResources.RuntimeLibraryPath(FileName, Src);
			RuntimeDependencies.Add(Target, Source);
		}
	}

	private void AddLinkLibrary(string Name, LibSource Src)
	{
		List<string> FilesToAdd = new List<string>();

		if (Name.Contains("*"))
		{
			// Find all files matching the given pattern.
			FilesToAdd = FindMatchingFiles(PackagedAgxResources.LinkLibraryDirectory(Src), Name);
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
			PublicAdditionalLibraries.Add(PackagedAgxResources.LinkLibraryPath(FileName, Src));
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

	private string GetPackagedAgxResourcesPath()
	{
		return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "..", "Binaries", "ThirdParty", "agx"));
	}

	// Returns true if AGX Dynamics resources are currently packaged with the plugin.
	// Returns false otherwise.
	private bool IsAgxResourcesPackaged()
	{
		return Directory.Exists(GetPackagedAgxResourcesPath());
	}

	private void PackageAgxResources()
	{
		if (!Heuristics.IsAgxSetupEnvCalled())
		{
			Console.Error.WriteLine("Could not package AGX Dynamics resources because no AGX Dynamics installation "
				+ "was found. Please ensure that setup_env has been called.");
			return;
		}

		// TODO: copy all necessary files from AGX Dynamics to the plugin.

	}

	private class Heuristics
	{
		public static bool IsAgxSetupEnvCalled()
		{
			return Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR") != null;
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

	private class AgxResourcesInfo
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

		public string RuntimeLibraryPath(string LibraryName, LibSource Src,
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
				return Path.Combine(Info.RuntimeLibrariesPath, LibraryName);
			}
			else
			{
				return Path.Combine(Info.RuntimeLibrariesPath, RuntimeLibraryFileName(LibraryName));
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

		private string GetComponentsRuntimePath(ReadOnlyTargetRules Target, string PackagedAgxResourcesPath,
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
					return Path.Combine(PackagedAgxResourcesPath, "plugins", "Components");
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
					return Path.Combine(PackagedAgxResourcesPath, "plugins", "Components");
				}
			}
			else
			{
				Console.WriteLine("Could not get Components runtime path from unsupported platform: " +
					Target.Platform);
				return string.Empty;
			}
		}

		public AgxResourcesInfo(ReadOnlyTargetRules Target, AgxResourcesLocation AgxLocation, string PackagedAgxResourcesPath = "")
		{
			LibSources = new Dictionary<LibSource, LibSourceInfo>();
			bool UseInstalledAgx = AgxLocation == AgxResourcesLocation.InstalledAgx ? true : false;

			if (UseInstalledAgx && !Heuristics.IsAgxSetupEnvCalled())
			{
				Console.Error.WriteLine("Tried to create an AgxResourcesInfo instance with installed AGX, but " +
					"setup_env has not been called.");
				return;
			}

			// TODO: Detect if AGX Dynamics is in local build or installed mode.
			//	   Currently assuming local build for Linux and installed for Windows.
			string BaseDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_DIR") : PackagedAgxResourcesPath;
			string BuildDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_BUILD_DIR")
				?? BaseDir : PackagedAgxResourcesPath;
			string DependenciesDir = UseInstalledAgx ? Environment.GetEnvironmentVariable("AGX_DEPENDENCIES_DIR")
				?? BaseDir : PackagedAgxResourcesPath;
			string TerrainDependenciesDir = UseInstalledAgx ?
				Environment.GetEnvironmentVariable("AGXTERRAIN_DEPENDENCIES_DIR")
				?? BaseDir : PackagedAgxResourcesPath;
			if (UseInstalledAgx && (BaseDir == null || BuildDir == null || DependenciesDir == null
				|| TerrainDependenciesDir == null))
			{
				Console.Error.WriteLine("Did not find AGX Dynamics installation folder.");
				Console.Error.WriteLine("Please check that your AGX Dynamics installation is valid.");
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
						: Path.Combine(BaseDir, "data", "cfg")
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
						: Path.Combine(BaseDir, "bin", "Win64")
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
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "bin", "Win64")
				));

				LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
					Path.Combine(BaseDir, "include"),
					UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64"),
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "bin", "Win64")
				));

				LibSources.Add(LibSource.Cfg, new LibSourceInfo(
					null,
					null,
					UseInstalledAgx ? Path.Combine(Environment.GetEnvironmentVariable("AGX_DATA_DIR"), "cfg")
						: Path.Combine(BaseDir, "agx", "data", "cfg")
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
