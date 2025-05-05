// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class DrivingTarget : TargetRules
{
	public DrivingTarget(TargetInfo Target) : base(Target)
	{
        bOverrideBuildEnvironment = true;
        //DefaultBuildSettings = BuildSettingsVersion.Latest;
        Type = TargetType.Game;
		//DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "Driving" } );
	}
}
