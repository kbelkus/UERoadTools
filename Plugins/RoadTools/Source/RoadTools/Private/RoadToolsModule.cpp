// Copyright Epic Games, Inc. All Rights Reserved.

#include "RoadToolsModule.h"
#include "RoadToolsEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "RoadToolsModule"

void FRoadToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FRoadToolsEditorModeCommands::Register();
}

void FRoadToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FRoadToolsEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRoadToolsModule, RoadToolsEditorMode)