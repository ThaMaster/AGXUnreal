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
			"AGXUnrealBarrier", "RHI", "RenderCore", "Core", "CoreUObject", "Engine", "InputCore", "Niagara"});


		// TODO: Determine which of these are really needed and why.
		// TODO: Why are some modules listed both here and in Public... above?
		// TODO: These were included before. Don't know why or where they came from. Remove?
		//       "CoreUObject", "Engine", "Slate", "SlateCore"
		PrivateDependencyModuleNames.AddRange(new string[] {
			"RHI", "RenderCore", "Projects", "Json", "Landscape", "Slate",
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
			Console.WriteLine("AGXUnreal: Git not installed, cannot get plugin revision, branch, or tag.");
			return;
		}

		string RepositoryPath = GetPluginRootPath();

		// When running on GitLab CI the working copy is created by GitLab but
		// this script is run by the runner's user. This means that the file
		// ownership isn't what Git expects, resulting in the following error:
		//     detected dubious ownership in repository at PATH.
		// The following tells Git that we are OK with executing binaries in
		// this directory.
		//
		// No error checking on this one, if it works then it works and if it
		// fails than later commands will fail with a descriptive error message.
		//
		// Hard-coded path for now, may need to do something better eventually.
		string SafeDirArgs = "config --local --add safe.directory /builds/algoryx/unreal/agxunreal";
		RunProcess("git", SafeDirArgs);



		/// TODO: Move the above to Dockerfile, don't want to do that on user's machines.



		// Here we want to determine if we are running inside the AGX Dynamics
		// for Unreal Git repository or not. A somewhat complicating matter is
		// that when running on GitLab CI we don't have a full working copy of
		// the repository and we don't have a proper branch checked out. So many
		// of the Git commands we are used to doesn't work. For example,
		//     git tag --list
		// doesn't list all tags.
		//
		// We determine if we are in the AGX Dynamics for Unreal repository
		// by checking the name of the remote.
		string RemoteArgs = String.Format("-C \"{0}\" remote -v", RepositoryPath);
		ProcessResult RemoteResult = RunProcess("git", RemoteArgs);
		if (RemoteResult.Success)
		{
			Console.WriteLine("AGXUnreal: Remote: {0}", RemoteResult.Output);
		}
		if (!RemoteResult.Success)
		{
			Console.WriteLine("AGXUnreal: Could not get Git remote: {0}", RemoteResult.Error);
			return;
		}
		if (!RemoteResult.Output.Contains("algoryx/unreal/agxunreal.git"))
		{
			// Not in an AGX Dynamics for Unreal working copy.
			Console.WriteLine("AGXUnreal: Not in an AGX Dynamics for Unreal working copy:");
			Console.WriteLine("AGXUnreal:    {0}", RemoteResult.Output);
			return;
		}


		// string AGXUnrealTagArgs = String.Format("-C \"{0}\" tag --list agxunreal-*", RepositoryPath);
		// ProcessResult HasTagsResult = RunProcess("git", AGXUnrealTagArgs);
		// if (HasTagsResult.Success)
		// {
		// 	Console.WriteLine("AGXUnreal: AGXUnreal tags:");
		// 	Console.WriteLine(HasTagsResult.Output);
		// }
		// if (!HasTagsResult.Success || String.IsNullOrEmpty(HasTagsResult.Output))
		// {
		// 	// We are not in the AGXUnreal repo, do not read or overwrite git info.
		// 	Console.WriteLine("AGXUnreal: Did not find any AGX Dynamics for Unreal tags in {0}, assuming not in the AGXUnreal repository.", RepositoryPath);
		// 	Console.WriteLine("AGXUnreal:    {0}", HasTagsResult.Error);
		// 	return;
		// }

		// Get Git hash for the current commit.
		string Hash;
		string GetHashArgs = String.Format("-C \"{0}\" rev-parse HEAD", RepositoryPath);
		ProcessResult HashResult = RunProcess("git", GetHashArgs);
		if (HashResult.Success)
		{
			Console.WriteLine("AGXUnreal: GetHash output:");
			Console.WriteLine(HashResult.Output);
			Hash = HashResult.Output.Trim();
		}
		else
		{
			Console.Error.WriteLine("Failed to get Git commit hash: \"{0}\"", HashResult.Error);
			Hash = "";
		}

		// Get current Git branch.
		string Branch;
		string GetBranchArgs = String.Format("-C \"{0}\" rev-parse --abbrev-ref HEAD", RepositoryPath);
		ProcessResult BranchResult = RunProcess("git", GetBranchArgs);
		if (BranchResult.Success)
		{
			Console.WriteLine("AGXUnreal: GetBranch output:");
			Console.WriteLine(BranchResult.Output);
			Branch = BranchResult.Output.Trim();
		}
		else
		{
			Console.Error.WriteLine("Failed to get Git branch: \"{0}\"", BranchResult.Error);
			Branch = "";
		}

		// When run in GitLab CI the above way of getting the branch name may
		// fail. This is a fallback way of getting the same information.
		if (Branch == "")
		{
			string DescribeArgs = String.Format("-C \"{0}\" describe --all", RepositoryPath);
			ProcessResult DescribeResult = RunProcess("git", DescribeArgs);
			if (DescribeResult.Success)
			{
				Console.WriteLine("AGXUnreal: Describe output:");
				Console.WriteLine(DescribeResult.Output);
				// This contains not only the branch name, but also a prefix
				// describing where the name was found. We many chose to do some
				// parsing of this, possibly to replace the tag read and possibly
				// to make sure we really got a branch. Or we could just remove
				// it
				Branch = DescribeResult.Output.Trim();
			}
			else
			{
				Console.WriteLine("AGXUnreal: Could not branch name using 'git describe':");
				Console.WriteLine("AGXUnreal:    {0}", DescribeResult.Error);
			}
		}

		if (Branch == "HEAD")
		{
			// Branch name is reported as HEAD when we are on a tag. Set it to empty string to signal no branch.
			Console.WriteLine("AGXUnreal: Branch is HEAD, which is not a valid branch name. Clearing.");
			Branch = "";
		}

		// Get the current Git tag, because git rev-parse doesn't identify branches.
		string Tag;
		string GetTagArgs = String.Format("-C \"{0}\" tag --points-at HEAD", RepositoryPath);
		ProcessResult TagResult = RunProcess("git", GetTagArgs);
		if (TagResult.Success)
		{
			Console.WriteLine("AGXUnreal: GetTag output:");
			Console.WriteLine(TagResult.Output);
			Tag = TagResult.Output.Trim();
		}
		else
		{
			Console.Error.WriteLine("Failed to get Git tag: \"{0}\"", TagResult.Error);
			Tag = "";
		}

		Console.WriteLine("AGXUnreal: Tag={1}, Branch={1}, Revision={2}", Tag, Branch, Hash);

		WriteGitInfo(Hash, Branch, Tag);
	}
}
