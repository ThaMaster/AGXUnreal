// Copyright 2023, Algoryx Simulation AB.

using System; // For Console, Environment.
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

		public bool IsValid()
		{
			return Success && Output.Trim() != "";
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

	private void WriteGitInfo(string Hash, string Name)
	{
		List<string> GitInfo = new List<string>();
		GitInfo.Add(String.Format("#define AGXUNREAL_HAS_GIT_HASH {0}", (Hash != "" ? "1" : "0")));
		GitInfo.Add(String.Format("const TCHAR* const AGXUNREAL_GIT_HASH = TEXT(\"{0}\");\n", Hash));

		GitInfo.Add(String.Format("#define AGXUNREAL_HAS_GIT_NAME {0}", (Name != "" ? "1" : "0")));
		GitInfo.Add(String.Format("const TCHAR* const AGXUNREAL_GIT_NAME = TEXT(\"{0}\");\n", Name));

		string FilePath = Path.Combine(GetPluginRootPath(), "Source", "AGXUnrealBarrier", "Public", "AGX_BuildInfo.generated.h");
		File.WriteAllLines(FilePath, GitInfo);
	}

	private string GitArgs(string Args)
	{
		string RepositoryPath = GetPluginRootPath();
		return String.Format("-C \"{0}\" {1}", RepositoryPath, Args);
	}

	private string RemovePrefix(string Name, string Prefix)
	{
		return Name.StartsWith(Prefix) ? Name.Substring(Prefix.Length) : Name;
	}

	private void CreateGitInfo()
	{
		// Don't write Git info if we can't run 'git'.
		// Not writing empty because if the file already exists then we assume
		// that it came from a plugin package built elsewhere and already
		// contains Git info.
		ProcessResult TestGitResult = RunProcess("git", "--version");
		if (!TestGitResult.Success)
		{
			/// TODO: Remove trace output.
			Console.WriteLine("AGXUnreal: Git not installed, cannot get plugin revision, branch, or tag.");

			return;
		}

		string RepositoryPath = GetPluginRootPath();


		// TODO: Remove the below once we have new Docker images.


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
		string SafeDirArgs = "config --global --add safe.directory /builds/algoryx/unreal/agxunreal";
		RunProcess("git", SafeDirArgs);



		/// TODO: Move the above to Dockerfile, don't want to do that on user's machines.



		// Determine if we are in an AGX Dynamics for Unreal working copy by
		// checking the name of the remote. If we aren't then we assume we
		// are in a client repository, or none at all, and assume that Git info
		// has either already been provided by the plugin package or isn't
		// necessary.
		ProcessResult GetRemoteResult = RunProcess("git", GitArgs("remote -v"));

		/// TODO: Remove trace output.
		if (GetRemoteResult.Success)
		{
			Console.WriteLine("AGXUnreal: Remote: {0}", GetRemoteResult.Output);
		}

		if (!GetRemoteResult.IsValid())
		{
			/// TODO: Remove trace output.
			Console.WriteLine("AGXUnreal: Could not get Git remote:");
			Console.WriteLine(GetRemoteResult.Error);

			return;
		}
		if (!GetRemoteResult.Output.Contains("algoryx/unreal/agxunreal.git"))
		{
			/// TODO: Remove trace output.
			Console.WriteLine("AGXUnreal: Not in an AGX Dynamics for Unreal working copy:");
			Console.WriteLine("AGXUnreal:    {0}", GetRemoteResult.Output);

			// Not in an AGX Dynamics for Unreal working copy.
			return;
		}


		// Get Git hash for the current commit.
		string Hash;
		ProcessResult GetHashResult = RunProcess("git", GitArgs("rev-parse HEAD"));
		if (GetHashResult.IsValid())
		{
			/// TODO: Remove trace output.
			Console.WriteLine("AGXUnreal: GetHash output:");
			Console.WriteLine(GetHashResult.Output);

			Hash = GetHashResult.Output.Trim();
		}
		else
		{
			Console.WriteLine("Failed to get Git commit hash:");
			Console.WriteLine(GetHashResult.Error);
			Hash = "";
		}


		// Get the name for this revision. This is either a tag or branch name.
		// Tag names get precedence over branch names, if both are available. A
		// GitLab CI provided name, through CI_COMMIT_REF_NAME, get precendence
		// over everything else since when running in GitLab CI we get bogus
		// branch names from Git itself.
		string Name = "";

		// First check CI_COMMIT_REF_NAME.
		if (String.IsNullOrEmpty(Name))
		{
			Name = Environment.GetEnvironmentVariable("CI_COMMIT_REF_NAME");
		}

		// Then try to read tag name from Git.
		if (String.IsNullOrEmpty(Name))
		{
			// An alternative is to use 'git describe --tags', not sure what the
			// pros and cons are.
			ProcessResult GetTagResult = RunProcess("git", GitArgs("tag --points-at HEAD"));
			if (GetTagResult.IsValid())
			{
				/// TODO: Remove trace output.
				Console.WriteLine("AGXUnreal: GetTag output:");
				Console.WriteLine(GetTagResult.Output);

				Name = GetTagResult.Output.Trim();
			}
			else
			{
				Console.WriteLine("Failed to get Git tag:");
				Console.WriteLine(GetTagResult.Error);
			}
		}

		// Then try to read branch name from Git with 'rev-parse'.
		if (String.IsNullOrEmpty(Name))
		{
			// Get current Git branch.
			ProcessResult GetBranchResult = RunProcess("git", GitArgs("rev-parse --abbrev-ref HEAD"));
			if (GetBranchResult.IsValid())
			{
				/// TODO: Remove trace output.
				Console.WriteLine("AGXUnreal: GetBranch output:");
				Console.WriteLine(GetBranchResult.Output);

				Name = GetBranchResult.Output.Trim();
			}
			else
			{
				Console.Error.WriteLine("Failed to get Git branch:");
				Console.WriteLine(GetBranchResult.Error);
			}

			// HEAD is not a valid branch name, we get this if there is no
			// current branch.
			if (Name == "HEAD")
			{
				Name = "";
			}
		}

		// Then try to read the branch or tag name from Git with 'describe`.
		if (String.IsNullOrEmpty(Name))
		{
			ProcessResult DescribeResult = RunProcess("git", GitArgs("describe --all"));
			if (DescribeResult.IsValid())
			{
				/// TODO: Remove trace output.
				Console.WriteLine("AGXUnreal: Describe output:");
				Console.WriteLine(DescribeResult.Output);

				// This contains not only the branch or tag name, but also a
				// prefix describing where the name was found, e.g. 'heads/' or
				// 'tags/'. Remove that part. This list may be incomplete.
				Name = DescribeResult.Output.Trim();
				Name = RemovePrefix(Name, "heads/");
				Name = RemovePrefix(Name, "tags/");
			}
			else
			{
				Console.WriteLine("AGXUnreal: Could not branch name using 'git describe':");
				Console.WriteLine(DescribeResult.Error);
			}
		}

		// HEAD is not a valid tag or branch name but can still show up if
		// name detection fails.
		if (Name == "HEAD")
		{
			/// TODO: Remove trace output.
			Console.WriteLine("AGXUnreal: Branch is HEAD, which is not a valid branch name. Clearing.");

			Name = "";
		}

		if (Name.StartsWith("pipelines/"))
		{
			/// TODO: Remove trace output.
			Console.WriteLine("AGXUnreal: Name starts with 'pipelines/' , which is not a valid branch name. Clearing.");

			// When running a GitLab pipeline we get a non-descript branch name
			// which should not be presented to the user. We should not get here,
			// in this case we should use CI_COMMIT_REF_NAME directly.
			Name = "";
		}

		Console.WriteLine("AGXUnreal: Hash={0}, Name={1}", Hash, Name);

		WriteGitInfo(Hash, Name);
	}
}
