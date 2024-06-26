// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class GGPOSteam : ModuleRules
{
    const string STEAMWORKS_VERSION = "157";
    const int STEAM_APP_ID = 480;

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
                "OnlineSubsystem",
                "OnlineSubsystemSteam",
                "Steamworks",
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

        EnsureSteamAppFile(STEAM_APP_ID);
    }

    private string PluginRootPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../")); }
    }
    private string ProjectRootPath
    {
        get { return Path.GetFullPath(Path.Combine(PluginRootPath, "../../")); }
    }
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ProjectRootPath, "ThirdParty/")); }
    }
    private string ExternalPath
    {
        get { return Path.GetFullPath(Path.Combine(ProjectRootPath, "External/")); }
    }
    private string SteamworksPath
    {
        get
        {
            string EngineDir = Path.GetFullPath(Target.RelativeEnginePath);
            return Path.Combine(EngineDir, "Source", "ThirdParty", "Steamworks", $"Steamv{STEAMWORKS_VERSION}", "sdk");

            //return Path.Combine(ExternalPath, "ggpo/external/steamworks/sdk");
        }
    }

    protected void AddGGPO()
    {
        bool bDebug = Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame;
        bool bDevelopment = Target.Configuration == UnrealTargetConfiguration.Development;

        string BuildFolder = bDebug ? "Debug" : "Release";

        // Library path
        string LibrariesPath = Path.Combine(ExternalPath, "ggpo/build/lib/x64", BuildFolder);

        string GGPOLib = Path.Combine(LibrariesPath, "GGPO.lib");
        string GGPOInclude = Path.Combine(ExternalPath, "ggpo/src/include");

        //System.Console.WriteLine("GGPO Library Path: " + GGPOLib);
        //System.Console.WriteLine("GGPO Include Path: " + GGPOInclude);

        PublicAdditionalLibraries.Add(GGPOLib);
        PublicIncludePaths.Add(GGPOInclude);

        // --------------------------------------------------------------
        // Build GGPO
        // --------------------------------------------------------------
        System.Console.WriteLine("Building GGPO library...");

        string BuildScriptPath = Path.Combine(PluginRootPath, "BuildGGPO.ps1");
        string BuildScriptConfig = bDebug ? "Debug" : "Release";
        string BuildCommand = $"powershell -ExecutionPolicy Bypass -File \"{BuildScriptPath}\" -BuildConfig \"{BuildScriptConfig}\" -SteamworksPath \"{SteamworksPath}\"";

        System.Console.WriteLine(BuildCommand);

        // Execute the PowerShell command
        var StartInfo = new System.Diagnostics.ProcessStartInfo("cmd.exe", "/c " + BuildCommand);
        var Process = new System.Diagnostics.Process();
        Process.StartInfo = StartInfo;
        Utils.RunLocalProcess(Process);

        System.Console.WriteLine("GGPO library build complete.");
        // --------------------------------------------------------------
    }

    /// <summary>
    /// Puts steam_appid.txt in the binaries directory if it doesn't exist. Defaults to 480 (Spacewar) if not specified.
    /// </summary>
    void EnsureSteamAppFile(int AppID = 480)
    {
        string BinariesDir = Path.Combine(ProjectRootPath, "Binaries/Win64");
        string SteamAppIDFile = Path.Combine(BinariesDir, "steam_appid.txt");

        if (!File.Exists(SteamAppIDFile))
        {
            if (!Directory.Exists(BinariesDir))
            {
                Directory.CreateDirectory(BinariesDir);
            }

            File.WriteAllText(SteamAppIDFile, AppID.ToString());
        }
    }
}
