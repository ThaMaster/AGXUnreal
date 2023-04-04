// Copyright 2023, Algoryx Simulation AB.

using System; // For Console.
using System.IO; // For Path.
using System.Diagnostics; // For running processes.
using System.Collections.Generic; // For List.

using UnrealBuildTool;

public class AGXUnreal : ModuleRules
{
	public AGXUnreal(ReadOnlyTargetRules Target) : base(Target)
	{
		//
		Console.Write(typeof(string).Assembly.ImageRuntimeVersion);

		// At 4.25 we started getting warnings encouraging us to enable these
		// settings. At or around 4.26 Unreal Engine makes these settings the
		// default.
		// bLegacyPublicIncludePaths adds all subdirectories to the list of
		// include paths passed to the compiler. This makes it too big for many
		// IDEs and compilers. Setting it to false reduces the list but makes
		// it necessary to specify subdirectories in #include statements.
		// PCHUsage has to do with Pre-Compiled Headers and include-what-you-use.
		// See
		// https://docs.unrealengine.com/4.26/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/IWYU/
		bLegacyPublicIncludePaths = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrecompileForTargets = PrecompileTargetsType.Any;

		// TODO: Determine which of these are really need and why.
		PublicDependencyModuleNames.AddRange(new string[] {
			"RHI", "RenderCore", "Core", "CoreUObject", "Engine", "InputCore", "Niagara"});


		// TODO: Determine which of these are really needed and why.
		// TODO: Why are some modules listed both here and in Public... above?
		// TODO: These were included before. Don't know why or where they came from. Remove?
		//       "CoreUObject", "Engine", "Slate", "SlateCore"
		PrivateDependencyModuleNames.AddRange(new string[] {
			"RHI", "RenderCore", "Projects", "Json", "AGXUnrealBarrier", "Landscape", "Slate",
			"SlateCore"});


		UpdateEngineVersionInUPlugin();
		CreateGitInfo();
	}

	/// Overwrite the EngineVersion attribute in AGXUnreal.uplugin with the
	/// Major.Minor version of the currently running Unreal Engine.
	private void UpdateEngineVersionInUPlugin()
	{
		// I would like to use System.Text.Json, but it seems Unreal Engine 4.26
		// has a too old C#/.Net version. I get the following error message:
		//
		//   error CS0234: The type or namespace name `Json' does not exist in the namespace `System.Text'.
		//   Are you missing an assembly reference?
		//
		// Doing manual line parsing for now.

		string PluginRoot = GetPluginRootPath();
		string UPluginPath = Path.Combine(PluginRoot, "AGXUnreal.uplugin");
		string UPluginTmpPath = Path.Combine(PluginRoot, "AGXUnreal.uplugin.tmp");
		string ErrorPrefix = "While updating Engine Version in AGXUnreal.uplugin:";
		string ErrorSuffix = "AGXUnreal.uplugin is unchanged.";

		if (!File.Exists(UPluginPath))
		{
			Console.Error.WriteLine(
				"{0} Could not find the AGXUnreal.uplugin file. Looked in {1}. {2}",
				ErrorPrefix, UPluginPath, ErrorSuffix);
			return;
		}

		BuildVersion Version;
		if (!BuildVersion.TryRead(BuildVersion.GetDefaultFileName(), out Version))
		{
			Console.Error.WriteLine(
				"{0} Could not parse Unreal Engine version from {1}. {2}",
				ErrorPrefix, BuildVersion.GetDefaultFileName(), ErrorSuffix);
			return;
		}

		List<string> NewUPluginContents = new List<string>();
		foreach (string Line in File.ReadLines(UPluginPath))
		{
			string ToWrite;
			if (Line.Contains("\"EngineVersion\": "))
			{
				ToWrite = String.Format(
					"\t\"EngineVersion\": \"{0}.{1}.0\",",
					Version.MajorVersion, Version.MinorVersion);
			}
			else
			{
				ToWrite = Line;
			}
			NewUPluginContents.Add(ToWrite);
		}

		if (NewUPluginContents.Count == 0)
		{
			Console.Error.WriteLine(
				"{0} Did not get any file contents from {1}. {2}",
				ErrorPrefix, UPluginPath, ErrorSuffix);
			return ;
		}

		File.WriteAllLines(UPluginTmpPath, NewUPluginContents);
		// File.Move overload with Overwrite flag is not available in Unreal Engine 4.26.
		File.Delete(UPluginPath);
		File.Move(UPluginTmpPath, UPluginPath);
	}

	/// Return the path to the AGXUnreal subdirectory in the Plugins directory.
	private string GetPluginRootPath()
	{
		// ModuelDirectory is the full path to Plugins/AGXUnreal/Source/AGXUnreal.
		return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
	}

	private struct ProcessResult
	{
		public bool Success;
		public string Output;
		public string Error;

		public ProcessResult(bool success, string output, string error)
		{
			Success = success;
			Output = output;
			Error = error;
		}
	}

	private ProcessResult RunProcess(string Executable, string Arguments)
	{
		try
		{
			var Config = new ProcessStartInfo(Executable, Arguments)
			{
				CreateNoWindow = true,
				UseShellExecute = false,
				RedirectStandardOutput = true,
				RedirectStandardError = true
			};
			Process RunningProcess = Process.Start(Config);
			string Output = RunningProcess.StandardOutput.ReadToEnd();
			string Error = RunningProcess.StandardError.ReadToEnd();
			RunningProcess.WaitForExit();
			return new ProcessResult(RunningProcess.ExitCode == 0, Output, Error);
		}
		catch (Exception exception)
		{
			Console.WriteLine("Cannot run process '{0}': {1}", Executable, exception.Message);
			return new ProcessResult(false, "", "");
		}
	}

	private void WriteGitInfo(string Hash, string Branch, string Tag)
	{
		List<string> GitInfo = new List<string>();
		GitInfo.Add(String.Format("#define AGXUNREAL_HAS_GIT_HASH {0}", (Hash != "" ? "1" : "0")));
		GitInfo.Add(String.Format("const TCHAR* const AGXUNREAL_GIT_HASH = TEXT(\"{0}\");\n", Hash));
		GitInfo.Add(String.Format("#define AGXUNREAL_HAS_GIT_BRANCH {0}", (Branch != "" ? "1" : "0")));
		GitInfo.Add(String.Format("const TCHAR* const AGXUNREAL_GIT_BRANCH = TEXT(\"{0}\");\n", Branch));
		GitInfo.Add(String.Format("#define AGXUNREAL_HAS_GIT_TAG {0}", (Tag != "" ? "1" : "0")));
		GitInfo.Add(String.Format("const TCHAR* const AGXUNREAL_GIT_TAG = TEXT(\"{0}\");\n", Tag));

		string FilePath = Path.Combine(GetPluginRootPath(), "Source", "AGXUnrealBarrier", "Public", "AGX_BuildInfo.generated.h");
		File.WriteAllLines(FilePath, GitInfo);
	}

	private void CreateGitInfo()
	{
		// Write empty Git info if we can't run 'git'.
		ProcessResult TestGit = RunProcess("git", "--version");
		if (!TestGit.Success)
		{
			Console.WriteLine("Git is not available so cannot read revision information.");
			WriteGitInfo("", "", "");
			return;
		}

		string RepositoryPath = GetPluginRootPath();

		// Get Git hash for the current commit.
		string Hash;
		string GetHashArgs = String.Format("-C {0} rev-parse HEAD", RepositoryPath);
		ProcessResult HashResult = RunProcess("git", GetHashArgs);
		if (HashResult.Success)
		{
			Hash = HashResult.Output.Trim();
		}
		else
		{
			Console.Error.WriteLine("Failed to get Git commit hash: {0}", HashResult.Error);
			Hash = "";
		}

		// Get current Git branch.
		string Branch;
		string GetBranchArgs = String.Format("-C {0} rev-parse --abbrev-ref HEAD", RepositoryPath);
		ProcessResult BranchResult = RunProcess("git", GetBranchArgs);
		if (BranchResult.Success)
		{
			Branch = BranchResult.Output.Trim();
		}
		else
		{
			Console.Error.WriteLine("Failed to get Git branch: {0}", BranchResult.Error);
			Branch = "";
		}
		if (Branch == "HEAD")
		{
			// Branch name is reported as HEAD when we are on a tag. Set it to empty string to signal no branch.
			Branch = "";
		}

		// Get the current Git tag, because git rev-parse doesn't identify branches.
		string Tag;
		string GetTagArgs = String.Format("-C {0} tag --points-at HEAD", RepositoryPath);
		ProcessResult TagResult = RunProcess("git", GetTagArgs);
		if (TagResult.Success)
		{
			Tag = TagResult.Output.Trim();
		}
		else
		{
			Console.Error.WriteLine("Failed to get Git tag: {0}", TagResult.Error);
			Tag = "";
		}

		WriteGitInfo(Hash, Branch, Tag);
	}
}
