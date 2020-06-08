// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class DQN_Car : ModuleRules
{
	private string projectRootPath
	{
		get { return Path.Combine(ModuleDirectory, "../../Thirdparty"); }
	}

	public DQN_Car(ReadOnlyTargetRules Target) : base(Target)
	{
		bUseRTTI = true;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });


		string libtorch_include = "libtorch/include";
		string libtorch_torch_include = "libtorch/include/torch/csrc/api/include";
		string libtorch_lib = "libtorch/lib";

		PublicIncludePaths.Add(Path.Combine(projectRootPath, libtorch_include));
		PublicIncludePaths.Add(Path.Combine(projectRootPath, libtorch_torch_include));

		PublicAdditionalLibraries.Add(Path.Combine(projectRootPath, libtorch_lib, "c10.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(projectRootPath, libtorch_lib, "torch.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(projectRootPath, libtorch_lib, "caffe2_module_test_dynamic.lib"));

		string cuda_path = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.1";
		string cuda_include = "include";
		string cuda_lib = "lib/x64";

		PublicIncludePaths.Add(Path.Combine(cuda_path, cuda_include));
		PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "cudart_static.lib"));


		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
