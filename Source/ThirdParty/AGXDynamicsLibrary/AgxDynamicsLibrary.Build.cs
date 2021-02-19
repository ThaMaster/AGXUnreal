using System.IO;
using System;
using System.Collections.Generic;
using UnrealBuildTool;


/// The AGXDynamicsLibrary is the portal from AGXUnrealBarrier to AGX Dynamics.
/// This is where we list the include paths and linker requirements to build an
/// Unreal Engine plugin that uses AGX Dynamics.
public class AGXDynamicsLibrary : ModuleRules
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
		Cfg,
		TerrainMaterialLibrary
	};

	private enum AgxResourcesLocation {
		InstalledAgx,
		PackagedAgx
	}

	// All needed AGX Dynamics runtime and build-time resources are located in the
	// directory AGXUnreal/Binaries/Thirdparty/agx within this plugin.
	// When compiling/packaging the AGX Dynamics for Unreal plugin, the AGX Dynamics
	// resources found in AGXUnreal/Binaries/Thirdparty/agx are used. In this
	// directory also all needed AGX Dynamics runtime files are located. That means
	// this plugin can be built and used without the need to call AGX Dynamic's
	// setup_env as long as these resources are available.
	//
	// If the AGX Dynamics resources are not available in the directory
	// AGXUnreal/Binaries/Thirdparty/agx, they are automatically copied from the
	// AGX Dynamics installation who's setup_env has been called as part of the
	// build process. Note that this means that if the AGX Dynamics resources are
	// not available in AGXUnreal/Binaries/Thirdparty/agx at build-time, setup_env
	// must have been called prior to performing the build.

	// Important note:
	// Currently no reliable way of copying the AGX Dynamics resources to the correct
	// location when building an executable from a project using this plugin has been
	// found. It is therefore recommended to manually copy the 'agx' directory located
	// in AGXUnreal/Binaries/Thirdparty and place it in the same directory as the
	// executable file. Also, all files inside
	// AGXUnreal/Binaries/Thirdparty/agx/bin/<platform> should be manually copied to
	// the same directory as the executable file.

	public AGXDynamicsLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		string PackagedAgxResoucesPath = GetPackagedAgxResourcesPath();
		PackagedAgxResources =
			new AgxResourcesInfo(Target, AgxResourcesLocation.PackagedAgx, PackagedAgxResoucesPath);
		Type = ModuleType.External;

		// Because the AGX Dynamics type system uses typeid and dynamic_cast.
		bUseRTTI = true;

		// Because AGX Dynamics uses exceptions.
		bEnableExceptions = true;

		Dictionary<string, LibSource> RuntimeLibFiles = new Dictionary<string, LibSource>();
		Dictionary<string, LibSource> LinkLibFiles = new Dictionary<string, LibSource>();
		List<LibSource> IncludePaths = new List<LibSource>();

		RuntimeLibFiles.Add("agxPhysics", LibSource.Agx);
		RuntimeLibFiles.Add("agxCore", LibSource.Agx);
		RuntimeLibFiles.Add("agxSabre", LibSource.Agx);
		RuntimeLibFiles.Add("agxTerrain", LibSource.Agx);
		RuntimeLibFiles.Add("agxCable", LibSource.Agx);
		RuntimeLibFiles.Add("agxModel", LibSource.Agx);
		RuntimeLibFiles.Add("vdbgrid", LibSource.Agx);
		RuntimeLibFiles.Add("colamd", LibSource.Agx);
		RuntimeLibFiles.Add("zlib", LibSource.Dependencies);
		RuntimeLibFiles.Add("Half", LibSource.TerrainDependencies);
		RuntimeLibFiles.Add("openvdb", LibSource.TerrainDependencies);
		RuntimeLibFiles.Add("tbb", LibSource.TerrainDependencies);

		LinkLibFiles.Add("agxPhysics", LibSource.Agx);
		LinkLibFiles.Add("agxCore", LibSource.Agx);
		LinkLibFiles.Add("agxSabre", LibSource.Agx);
		LinkLibFiles.Add("agxTerrain", LibSource.Agx);
		LinkLibFiles.Add("agxCable", LibSource.Agx);
		LinkLibFiles.Add("agxModel", LibSource.Agx);

		IncludePaths.Add(LibSource.Agx);

		// OS specific dependencies.
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			RuntimeLibFiles.Add("png", LibSource.Dependencies);
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			RuntimeLibFiles.Add("msvcp140", LibSource.Agx);
			RuntimeLibFiles.Add("vcruntime140", LibSource.Agx);

			RuntimeLibFiles.Add("websockets", LibSource.Dependencies);
			RuntimeLibFiles.Add("libpng", LibSource.Dependencies);
			RuntimeLibFiles.Add("ot2?-OpenThreads", LibSource.Dependencies);
			RuntimeLibFiles.Add("glew", LibSource.Dependencies);
		}

		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			IncludePaths.Add(LibSource.Components);
			IncludePaths.Add(LibSource.Config);
			IncludePaths.Add(LibSource.Dependencies);
			IncludePaths.Add(LibSource.TerrainDependencies);
		}

		// Package AGX Dynamics resources in plugin if no packaged resources exists.
		if (!IsAgxResourcesPackaged())
		{
			PackageAgxResources(Target, RuntimeLibFiles, LinkLibFiles, IncludePaths);
		}
		else
		{
			Console.WriteLine("Skipping packaging of AGX Dynamics resources, packaged "
				+ "resources already exists in: {0}", PackagedAgxResoucesPath);
		}

		foreach (var RuntimeLibFile in RuntimeLibFiles)
		{
			AddRuntimeDependency(RuntimeLibFile.Key, RuntimeLibFile.Value, Target);
		}

		foreach (var LinkLibFile in LinkLibFiles)
		{
			AddLinkLibrary(LinkLibFile.Key, LinkLibFile.Value);
		}

		foreach (var HeaderPath in IncludePaths)
		{
			AddIncludePath(HeaderPath);
		}
	}

	private void AddIncludePath(LibSource Src)
	{
		PublicIncludePaths.Add(PackagedAgxResources.IncludePath(Src));
	}

	// The runtime dependency file is copied to the target binaries directory.
	private void AddRuntimeDependency(string Name, LibSource Src, ReadOnlyTargetRules Target)
	{
		string Dir = PackagedAgxResources.RuntimeLibraryDirectory(Src);
		string FileName = PackagedAgxResources.RuntimeLibraryFileName(Name);

		// File name and/or extension may include search patterns such as '*' or '?'. Resolve all these.
		string[] FilesToAdd = Directory.GetFiles(Dir, FileName);

		if (FilesToAdd.Length == 0)
		{
			Console.Error.WriteLine("Error: File {0} did not match any file in {1}. The dependency " +
				"will not be added in the build.", FileName, Dir);
			return;
		}

		foreach (string FilePath in FilesToAdd)
		{
			// This is a temporary work-around to fix the issue where dll/so files are copied to the
			// Binaries directory of any project that are built that uses this plugin, which should not happen.
			// See internal issue 282.
			// @todo Figure out how to properly copy the dll/so files for all build scenarios.
			if (ShouldCopyBinFiles(Target))
			{
				string Dest = Path.Combine("$(BinaryOutputDir)", Path.GetFileName(FilePath));
				RuntimeDependencies.Add(Dest, FilePath);
			}
			else
			{
				RuntimeDependencies.Add(FilePath);
			}
		}
	}

	private void AddLinkLibrary(string Name, LibSource Src)
	{
		string Dir = PackagedAgxResources.LinkLibraryDirectory(Src);
		string FileName = PackagedAgxResources.LinkLibraryFileName(Name);

		// File name and/or extension may include search patterns such as '*' or '?'. Resolve all these.
		string[] FilesToAdd = Directory.GetFiles(Dir, FileName);

		if (FilesToAdd.Length == 0)
		{
			Console.Error.WriteLine("Error: File {0} did not match any file in {1}. The library will not be added " +
				"in the build.", FileName, Dir);
			return;
		}

		foreach (string FilePath in FilesToAdd)
		{
			PublicAdditionalLibraries.Add(FilePath);
		}
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

	private void PackageAgxResources(ReadOnlyTargetRules Target, Dictionary<string, LibSource> RuntimeLibFiles,
		Dictionary<string, LibSource> LinkLibFiles, List<LibSource> IncludePaths)
	{
		if (!Heuristics.IsAgxSetupEnvCalled())
		{
			Console.Error.WriteLine("Error: Could not package AGX Dynamics resources because no AGX Dynamics installation "
				+ "was found. Please ensure that setup_env has been called.");
			return;
		}

		Console.WriteLine("Packaging AGX Dynamics resources starting...");
		AgxResourcesInfo InstalledAgxResources = new AgxResourcesInfo(Target, AgxResourcesLocation.InstalledAgx);

		// Copy AGX Dynamics runtime library files.
		foreach (var RuntimeLibFile in RuntimeLibFiles)
		{
			string Dir = InstalledAgxResources.RuntimeLibraryDirectory(RuntimeLibFile.Value);
			string FileName = InstalledAgxResources.RuntimeLibraryFileName(RuntimeLibFile.Key);

			// File name and/or extension may include search patterns such as '*' or '?'. Resolve all these.
			string[] FilesToCopy = Directory.GetFiles(Dir, FileName);
			if (FilesToCopy.Length == 0)
			{
				Console.Error.WriteLine("Error: File {0} did not match any file in {1}. Packaging " +
					"of AGX Dynamics resources failed.", FileName, Dir);
				CleanPackagedAgxDynamicsResources();
				return;
			}

			foreach (string FilePath in FilesToCopy)
			{
				// Note: the PackagedAgxResources.RuntimeLibraryPath() function cannot be used here since
				// the file name may have an added prefix that would then be added once again.
				string Dest = Path.Combine(
					PackagedAgxResources.RuntimeLibraryDirectory(RuntimeLibFile.Value), Path.GetFileName(FilePath));
				if (!CopyFile(FilePath, Dest))
				{
					CleanPackagedAgxDynamicsResources();
					return;
				}
			}
		}

		// Copy AGX Dynamics link library files.
		foreach (var LinkLibFile in LinkLibFiles)
		{
			string Dir = InstalledAgxResources.LinkLibraryDirectory(LinkLibFile.Value);
			string FileName = InstalledAgxResources.LinkLibraryFileName(LinkLibFile.Key);

			// File name and/or extension may include search patterns such as '*' or '?'. Resolve all these.
			string[] FilesToCopy = Directory.GetFiles(Dir, FileName);
			if (FilesToCopy.Length == 0)
			{
				Console.Error.WriteLine("Error: File {0} did not match any file in {1}. Packaging " +
					"of AGX Dynamics resources failed.", FileName, Dir);
				CleanPackagedAgxDynamicsResources();
				return;
			}

			foreach (string FilePath in FilesToCopy)
			{
				// Note: the PackagedAgxResources.LinkLibraryPath() function cannot be used here since
				// the file name may have an added prefix that would then be added once again.
				string Dest = Path.Combine(
					PackagedAgxResources.LinkLibraryDirectory(LinkLibFile.Value), Path.GetFileName(FilePath));
				if (!CopyFile(FilePath, Dest))
				{
					CleanPackagedAgxDynamicsResources();
					return;
				}
			}
		}

		// Copy AGX Dynamics header files.
		foreach (var IncludePath in IncludePaths)
		{
			string Source = InstalledAgxResources.IncludePath(IncludePath);
			string Dest = PackagedAgxResources.IncludePath(IncludePath);
			if(!CopyDirectoryRecursively(Source, Dest))
			{
				CleanPackagedAgxDynamicsResources();
				return;
			}
		}

		// Copy AGX Dynamics cfg directory.
		{
			string Source = InstalledAgxResources.RuntimeLibraryPath(string.Empty, LibSource.Cfg, true);
			string Dest = PackagedAgxResources.RuntimeLibraryPath(string.Empty, LibSource.Cfg, true);
			if (!CopyDirectoryRecursively(Source, Dest))
			{
				CleanPackagedAgxDynamicsResources();
				return;
			}
		}

		// Copy Terrain Material Library.
		{
			string Source = InstalledAgxResources.RuntimeLibraryPath(string.Empty, LibSource.TerrainMaterialLibrary, true);
			string Dest = PackagedAgxResources.RuntimeLibraryPath(string.Empty, LibSource.TerrainMaterialLibrary, true);
			if (!CopyDirectoryRecursively(Source, Dest))
			{
				CleanPackagedAgxDynamicsResources();
				return;
			}
		}

		// Copy AGX Dynamics Components/agx/Physics directory and Components/agx/Referenced.agxEntity file.
		{
			string ComponentsDirSource = InstalledAgxResources.RuntimeLibraryPath(string.Empty, LibSource.Components, true);
			string ComponentsDirDest = PackagedAgxResources.RuntimeLibraryPath(string.Empty, LibSource.Components, true);
			string PhysicsDirSource = Path.Combine(ComponentsDirSource, "agx", "Physics");
			string PhysicsDirDest = Path.Combine(ComponentsDirDest, "agx", "Physics");

			if (!CopyDirectoryRecursively(PhysicsDirSource, PhysicsDirDest))
			{
				CleanPackagedAgxDynamicsResources();
				return;
			}

			// Copy all single files in the Components/agx directory.
			if (!CopyFilesNonRecursive(Path.Combine(ComponentsDirSource, "agx"), Path.Combine(ComponentsDirDest, "agx")))
			{
				CleanPackagedAgxDynamicsResources();
				return;
			}
		}

		Console.WriteLine("Packaging AGX Dynamics resources complete.");
	}

	private bool CopyFile(string Source, string Dest)
	{
		try
		{
			string DestDir = Path.GetDirectoryName(Dest);
			if (!Directory.Exists(DestDir))
			{
				Directory.CreateDirectory(DestDir);
			}

			File.Copy(Source, Dest, true);
		}
		catch (Exception e)
		{
			Console.Error.WriteLine("Error: Unable to copy file {0} to {1}. Exception: {2}", Source, Dest, e.Message);
			return false;
		}

		return true;
	}

	private bool CopyDirectoryRecursively(string SourceDir, string DestDir)
	{
		foreach (string DirPath in Directory.GetDirectories(SourceDir, "*", SearchOption.AllDirectories))
		{
			Directory.CreateDirectory(DirPath.Replace(SourceDir, DestDir));
		}

		foreach (string FilePath in Directory.GetFiles(SourceDir, "*", SearchOption.AllDirectories))
		{
			// Do not copy license files.
			if (Path.GetExtension(FilePath).Equals(".lic"))
			{
				continue;
			}

			if (!CopyFile(FilePath, FilePath.Replace(SourceDir, DestDir)))
			{
				return false;
			}
		}

		return true;
	}

	private bool CopyFilesNonRecursive(string SourceDir, string DestDir)
	{
		string[] FilesPaths = Directory.GetFiles(SourceDir);

		foreach (string FilePath in FilesPaths)
		{
			string FileName = Path.GetFileName(FilePath);
			if (!CopyFile(FilePath, Path.Combine(DestDir, FileName)))
			{
				return false;
			}
		}

		return true;
	}

	private void CleanPackagedAgxDynamicsResources()
	{
		Console.WriteLine("Cleaning packaged AGX Dynamics resources started...");
		string PackagedAgxResourcesPath = GetPackagedAgxResourcesPath();
		try
		{
			if (Directory.Exists(PackagedAgxResourcesPath))
			{
				Directory.Delete(PackagedAgxResourcesPath, true);
			}
		}
		catch (Exception e)
		{
			Console.Error.WriteLine("Error: Unable to delete directory {0}. Exception: {1}",
				PackagedAgxResourcesPath, e.Message);
			return;
		}
		Console.WriteLine("Cleaning packaged AGX Dynamics resources complete.");
	}

	// This is a temporary work-around to fix the issue where dll/so files are copied to the
	// Binaries directory of any project that are built that uses this plugin, which should not happen.
	// See internal issue 282.
	// @todo Figure out how to properly copy the dll/so files for all build scenarios.
	private bool ShouldCopyBinFiles(ReadOnlyTargetRules Target)
	{
		string ProjectName = Path.GetFileNameWithoutExtension(Target.ProjectFile.ToString());

		// When building an executable, the project name will be set to 'HostProject'.
		return ProjectName.Equals("AGXUnrealDev") || ProjectName.Equals("HostProject");
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
				Console.Error.WriteLine("Error: No include path for '{0}'.", Src);
				return null;
			}
			return Info.IncludePath;
		}

		public string LinkLibraryPath(string LibraryName, LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.LinkLibrariesPath == null)
			{
				Console.Error.WriteLine("Error: No LinkLibraryPath for '{0}', '{1}' cannot be found.", Src, LibraryName);
				return LibraryName;
			}
			return Path.Combine(Info.LinkLibrariesPath, LinkLibraryFileName(LibraryName));
		}

		public string LinkLibraryDirectory(LibSource Src)
		{
			LibSourceInfo Info = LibSources[Src];
			if (Info.LinkLibrariesPath == null)
			{
				Console.Error.WriteLine("Error: No LinkLibraryPath for '{0}'.", Src);
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
				Console.Error.WriteLine("Error: No RuntimeLibraryPath for '{0}', '{1}' cannot be found.", Src,
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
				Console.Error.WriteLine("Error: No RuntimeLibraryDirectory for '{0}'.", Src);
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
				Console.Error.WriteLine("Error: Did not find AGX Dynamics installation folder.");
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
					UseInstalledAgx ? Path.Combine(BuildDir, "lib") : Path.Combine(BaseDir, "lib", "Linux"),
					UseInstalledAgx ? Path.Combine(BuildDir, "lib") : Path.Combine(BaseDir, "bin", "Linux")
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
					UseInstalledAgx ? Path.Combine(DependenciesDir, "lib") : Path.Combine(BaseDir, "lib", "Linux"),
					UseInstalledAgx ? Path.Combine(DependenciesDir, "lib") : Path.Combine(BaseDir, "bin", "Linux")
				));

				LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
					Path.Combine(TerrainDependenciesDir, "include"),
					UseInstalledAgx ? Path.Combine(TerrainDependenciesDir, "lib") : Path.Combine(BaseDir, "lib", "Linux"),
					UseInstalledAgx ? Path.Combine(TerrainDependenciesDir, "lib") : Path.Combine(BaseDir, "bin", "Linux")
				));

				// On Linux, environment variable 'AGX_DATA_DIR' is not always visible here when using
				//  a built Agx Dynamics. Therefore 'AGX_DIR' is used instead to find the cfg directory.
				LibSources.Add(LibSource.Cfg, new LibSourceInfo(
					null,
					null,
					UseInstalledAgx ?
						Path.Combine(Environment.GetEnvironmentVariable("AGX_DIR"), "data", "cfg")
						: Path.Combine(BaseDir, "data", "cfg")
				));

				LibSources.Add(LibSource.TerrainMaterialLibrary, new LibSourceInfo(
					null,
					null,
					UseInstalledAgx ?
						Path.Combine(Environment.GetEnvironmentVariable("AGX_DIR"), "data", "TerrainMaterials")
						: Path.Combine(BaseDir, "data", "TerrainMaterials")
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
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "bin", "Win64")
				));

				LibSources.Add(LibSource.Config, new LibSourceInfo(
					null,
					null,
					null
				));

				LibSources.Add(LibSource.Components, new LibSourceInfo(
					null,
					null,
					GetComponentsRuntimePath(Target, BaseDir, UseInstalledAgx)
				));

				LibSources.Add(LibSource.Dependencies, new LibSourceInfo(
					null,
					UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64"),
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "bin", "Win64")
				));

				LibSources.Add(LibSource.TerrainDependencies, new LibSourceInfo(
					null,
					UseInstalledAgx ? Path.Combine(BaseDir, "lib", "x64") : Path.Combine(BaseDir, "lib", "Win64"),
					UseInstalledAgx ? Path.Combine(BaseDir, "bin", "x64") : Path.Combine(BaseDir, "bin", "Win64")
				));

				LibSources.Add(LibSource.Cfg, new LibSourceInfo(
					null,
					null,
					UseInstalledAgx ?
						Path.Combine(Environment.GetEnvironmentVariable("AGX_DATA_DIR"), "cfg")
						: Path.Combine(BaseDir, "data", "cfg")
				));

				LibSources.Add(LibSource.TerrainMaterialLibrary, new LibSourceInfo(
					null,
					null,
					UseInstalledAgx ?
						Path.Combine(Environment.GetEnvironmentVariable("AGX_DATA_DIR"), "TerrainMaterials")
						: Path.Combine(BaseDir, "data", "TerrainMaterials")
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
