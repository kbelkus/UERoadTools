// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Driving : ModuleRules
{
	public Driving(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RoadTools", "UMG" });

		PrivateDependencyModuleNames.AddRange(new string[] { "RoadTools" });

        //PublicIncludePaths.AddRange(new string[] { System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "Source/Editor/Blutility/Private" });

        //PublicIncludePaths.AddRange(new string[] { "RoadTools/Public", "RoadTools/Classes" });

        // Uncomment if you are using Slate UI
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
