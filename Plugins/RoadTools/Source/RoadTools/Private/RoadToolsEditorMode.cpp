// Copyright Epic Games, Inc. All Rights Reserved.

#include "RoadToolsEditorMode.h"
#include "RoadToolsEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "RoadToolsEditorModeCommands.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/RoadToolsSimpleTool.h"
#include "Tools/RoadToolsInteractiveTool.h"

// step 2: register a ToolBuilder in FRoadToolsEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "RoadToolsEditorMode"

const FEditorModeID URoadToolsEditorMode::EM_RoadToolsEditorModeId = TEXT("EM_RoadToolsEditorMode");

FString URoadToolsEditorMode::SimpleToolName = TEXT("RoadTools_ActorInfoTool");
FString URoadToolsEditorMode::InteractiveToolName = TEXT("RoadTools_MeasureDistanceTool");


URoadToolsEditorMode::URoadToolsEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(URoadToolsEditorMode::EM_RoadToolsEditorModeId,
		LOCTEXT("ModeName", "RoadTools"),
		FSlateIcon(),
		true);
}


URoadToolsEditorMode::~URoadToolsEditorMode()
{
}


void URoadToolsEditorMode::ActorSelectionChangeNotify()
{
}

void URoadToolsEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FRoadToolsEditorModeCommands& SampleToolCommands = FRoadToolsEditorModeCommands::Get();

	RegisterTool(SampleToolCommands.SimpleTool, SimpleToolName, NewObject<URoadToolsSimpleToolBuilder>(this));
	RegisterTool(SampleToolCommands.InteractiveTool, InteractiveToolName, NewObject<URoadToolsInteractiveToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, SimpleToolName);
}

void URoadToolsEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FRoadToolsEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> URoadToolsEditorMode::GetModeCommands() const
{
	return FRoadToolsEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
