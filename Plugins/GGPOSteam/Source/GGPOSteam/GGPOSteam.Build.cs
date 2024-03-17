// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class GGPOSteam : ModuleRules
{
	public GGPOSteam(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        AddGGPO();
    }

    private string ProjectRootPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../../")); }
    }
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ProjectRootPath, "ThirdParty/")); }
    }
    private string ExternalPath
    {
        get { return Path.GetFullPath(Path.Combine(ProjectRootPath, "External/")); }
    }

    protected void AddGGPO()
    {
        bool bDebug = Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame;
        bool bDevelopment = Target.Configuration == UnrealTargetConfiguration.Development;

        string BuildFolder = bDebug ? "Debug" : "Release";

        // Library path
        string LibrariesPath = Path.Combine(ExternalPath, "ggpo/build/lib/x64", BuildFolder);
        PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "GGPO.lib"));

        PublicIncludePaths.Add(Path.Combine(ExternalPath, "ggpo/src/include"));

        // --------------------------------------------------------------
        // Build GGPO
        // --------------------------------------------------------------
        System.Console.WriteLine("Building GGPO library...");

        string BuildScriptPath = Path.Combine(ProjectRootPath, "spr.ps1");
        string BuildScriptConfig = bDebug ? "Debug" : "Release";
        string BuildCommand = $"powershell -ExecutionPolicy Bypass -File {BuildScriptPath} -RunFunction \"BuildGGPO\" -BuildConfig \"{BuildScriptConfig}\"";

        // Execute the PowerShell command
        var StartInfo = new System.Diagnostics.ProcessStartInfo("cmd.exe", "/c " + BuildCommand);
        var Process = new System.Diagnostics.Process();
        Process.StartInfo = StartInfo;
        Utils.RunLocalProcess(Process);

        System.Console.WriteLine("GGPO library build complete.");
        // --------------------------------------------------------------
    }
}
