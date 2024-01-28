// Copyright Epic Games, Inc. All Rights Reserved.

#include "RoadToolsEditorModeToolkit.h"
#include "RoadToolsEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "RoadToolsEditorModeToolkit"

FRoadToolsEditorModeToolkit::FRoadToolsEditorModeToolkit()
{
}

void FRoadToolsEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FRoadToolsEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FRoadToolsEditorModeToolkit::GetToolkitFName() const
{
	return FName("RoadToolsEditorMode");
}

FText FRoadToolsEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "RoadToolsEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
