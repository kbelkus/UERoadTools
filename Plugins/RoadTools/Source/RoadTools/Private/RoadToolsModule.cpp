// Copyright Epic Games, Inc. All Rights Reserved.

#include "RoadToolsModule.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "Tools/SignalManagerVisualizer.h"
#include "SignalControllerComponent.h"
#include "RoadToolsEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "RoadToolsModule"

void FRoadToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FRoadToolsEditorModeCommands::Register();


	if (GUnrealEd)
	{
		

		UE_LOG(LogTemp, Warning, TEXT("KIERAN: THIS IS CALLED"));

		const TSharedPtr<FSignalManagerVisualizer> SignalComponentVisualizer = MakeShareable(new FSignalManagerVisualizer());

		GUnrealEd->RegisterComponentVisualizer(USignalControllerComponent::StaticClass()->GetFName(), SignalComponentVisualizer);
		SignalComponentVisualizer->OnRegister();

	}

}

void FRoadToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FRoadToolsEditorModeCommands::Unregister();

	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(USignalControllerComponent::StaticClass()->GetFName());
	}

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRoadToolsModule, RoadToolsEditorMode)