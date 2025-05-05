// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class DrivingEditorTarget : TargetRules
{
	public DrivingEditorTarget(TargetInfo Target) : base(Target)
	{
        bOverrideBuildEnvironment = true;
        Type = TargetType.Editor;
		//DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "Driving" } );
	}
}
