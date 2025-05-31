// Fill out your copyright notice in the Description page of Project Settings.

#define LOCTEXT_NAMESPACE "FJunctionVisualiser"

#include "ComponentVisualisers/JunctionVisualiser.h"
#include "HitProxies.h"
#include "EnumRange.h"

struct HJunctionBaseVisProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HJunctionBaseVisProxy(const UActorComponent* InComponent)
		: HComponentVisProxy(InComponent, HPP_Wireframe)
	{}
};

struct HJunctionPointHitProxy : public HJunctionBaseVisProxy
{
	DECLARE_HIT_PROXY();

	HJunctionPointHitProxy(const UActorComponent* InComponent, int32 InTargetIndex, EJunctionCommandAction InCommandAction)
		: HJunctionBaseVisProxy(InComponent)
		, TargetIndex(InTargetIndex)
		, CommandAction(InCommandAction)
	{}

	int32 TargetIndex;
	EJunctionCommandAction CommandAction;


	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::GrabHand;
	}
};

struct HJunctionEndHitProxy : public HJunctionBaseVisProxy
{
	DECLARE_HIT_PROXY();

	HJunctionEndHitProxy(const UActorComponent* InComponent, int32 InTargetIndex, int32 InCommandIndex)
		: HJunctionBaseVisProxy(InComponent)
		, TargetIndex(InTargetIndex)
		, CommandIndex(InCommandIndex)
	{}

	int32 TargetIndex;
	int32 CommandIndex;

	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::GrabHand;
	}
};

struct HJunctionLaneWidth : public HJunctionBaseVisProxy
{
	DECLARE_HIT_PROXY();

	HJunctionLaneWidth(const UActorComponent* InComponent, int32 InTargetIndex, EJunctionCommandAction InCommandAction, int32 InLaneIndex, int32 InLeftRightSwitch)
		: HJunctionBaseVisProxy(InComponent)
		, TargetIndex(InTargetIndex)
		, CommandAction(InCommandAction)
		, LaneIndex(InLaneIndex)
		, LeftRightSwitch(InLeftRightSwitch)
	{}

	int32 TargetIndex;
	EJunctionCommandAction CommandAction;
	int32 LaneIndex;
	int32 LeftRightSwitch;

	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::GrabHand;
	}
};

struct HJunctionLaneTurningType : public HJunctionBaseVisProxy
{
	DECLARE_HIT_PROXY();

	HJunctionLaneTurningType(const UActorComponent* InComponent, int32 InTargetIndex, EJunctionCommandAction InCommandAction, int32 InLaneIndex, int32 InLeftRightSwitch)
		: HJunctionBaseVisProxy(InComponent)
		, TargetIndex(InTargetIndex)
		, CommandAction(InCommandAction)
		, LaneIndex(InLaneIndex)
		, LeftRightSwitch(InLeftRightSwitch)
	{}

	int32 TargetIndex;
	EJunctionCommandAction CommandAction;
	int32 LaneIndex;
	int32 LeftRightSwitch;

	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::GrabHand;
	}
};

struct HJunctionLaneDrivingType : public HJunctionBaseVisProxy
{
	DECLARE_HIT_PROXY();

	HJunctionLaneDrivingType(const UActorComponent* InComponent, int32 InTargetIndex, EJunctionCommandAction InCommandAction, int32 InLaneIndex, int32 InLeftRightSwitch)
		: HJunctionBaseVisProxy(InComponent)
		, TargetIndex(InTargetIndex)
		, CommandAction(InCommandAction)
		, LaneIndex(InLaneIndex)
		, LeftRightSwitch(InLeftRightSwitch)
	{}

	int32 TargetIndex;
	EJunctionCommandAction CommandAction;
	int32 LaneIndex;
	int32 LeftRightSwitch;

	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::GrabHand;
	}
};


IMPLEMENT_HIT_PROXY(HJunctionBaseVisProxy,    HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HJunctionPointHitProxy,   HJunctionBaseVisProxy);
IMPLEMENT_HIT_PROXY(HJunctionEndHitProxy,     HJunctionBaseVisProxy);
IMPLEMENT_HIT_PROXY(HJunctionLaneWidth,       HJunctionBaseVisProxy);
IMPLEMENT_HIT_PROXY(HJunctionLaneTurningType, HJunctionBaseVisProxy);
IMPLEMENT_HIT_PROXY(HJunctionLaneDrivingType, HJunctionBaseVisProxy);

//Register our Commands
class FJunctionVisualiserCommands : public TCommands<FJunctionVisualiserCommands>
{

public:

	FJunctionVisualiserCommands() : TCommands<FJunctionVisualiserCommands>
		(
			"RoadToolsJunctionVis", LOCTEXT("RoadToolsJunctionVis", "RoadToolsJunctionVis"), NAME_None, FAppStyle::GetAppStyleSetName()
		) {}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(UpdateLaneDrivingType, "Select Lane Driving Type", "Select Lane Driving Type", EUserInterfaceActionType::Button, FInputGesture());
		UI_COMMAND(UpdateLaneTurningRule, "Select Lane Turning Rule", "Select Lane Turning Rule", EUserInterfaceActionType::Button, FInputGesture());
		UI_COMMAND(RebuildLanes, "RebuildLanes", "RebuildLanes", EUserInterfaceActionType::Button, FInputGesture());
	}

public:
	/** Add Commands */
	TSharedPtr<FUICommandInfo> UpdateLaneDrivingType;
	TSharedPtr<FUICommandInfo> UpdateLaneTurningRule;
	TSharedPtr<FUICommandInfo> RebuildLanes;

};

JunctionVisualiser::JunctionVisualiser()
{
}

JunctionVisualiser::~JunctionVisualiser()
{
}

void JunctionVisualiser::OnRegister()
{
	UE_LOG(LogTemp, Log, TEXT("JunctionSurfaceVisuliser was registered"));

	FJunctionVisualiserCommands::Register();

	FJunctionVisualiserActions = MakeShareable(new FUICommandList);
}

void JunctionVisualiser::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UJunctionSurfaceComponent* JunctionSurfaceComponent = Cast<UJunctionSurfaceComponent>(Component);

	if (!JunctionSurfaceComponent)
	{
		return;
	}

	//Validate Selected Sate
	const TObjectPtr<AActor> OwnerActor = Component->GetOwner();
	const FVector JunctionLocation = OwnerActor->GetActorLocation();

	OwnedJunctionSurface = Cast<AJunctionSurface>(Component->GetOwner());

	if (!OwnedJunctionSurface)
	{
		UE_LOG(LogTemp, Error, TEXT("Junction Visualiser could not get parent actor exiting."));
		return;
	}

	TArray<FJunctionPoint> JunctionPoints = JunctionSurfaceComponent->GetJunctionPoints();

	//MOVE THIS TO REGISTER?
	JunctionPointIcon = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/TransformSquare.TransformSquare")));
	JunctionEndIcon =	Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/TransformSquareLeftRight.TransformSquareLeftRight")));
	
	LaneTypeIconNone =  Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/LaneTypeNone.LaneTypeNone")));
	LaneTypeIconDriving = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/LaneTypeDriving.LaneTypeDriving")));
	LaneTypeIconBicycle = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/LaneTypeBike.LaneTypeBike")));
	LaneTypeIconShoulder = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/TransformSquareLeftRight.TransformSquareLeftRight")));

	LaneTurningLeft = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/LaneLeft.LaneLeft")));
	LaneTurningForward = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/LaneForward.LaneForward")));
	LaneTurningLeftForward = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/LaneForwardLeft.LaneForwardLeft")));
	LaneTurningAll = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/LaneAll.LaneAll")));
	LaneTurningForwardRight = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/LaneForwardRight.LaneForwardRight")));
	LaneTurningRight = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/RoadTools/Icons/LaneRight.LaneRight")));

	//Draw Junction Center UI


	//Draw Junction Location UI
	for (int i = 0; i < JunctionPoints.Num(); i++)
	{
		FVector DrawPointLocation = JunctionLocation + JunctionPoints[i].Location;
		FVector LaneEndLocation = JunctionPoints[i].EndLocation;

		//Location
		PDI->SetHitProxy(new HJunctionPointHitProxy(Component, i, EJunctionCommandAction::JUNCTIONPOINT));
		PDI->DrawSprite(DrawPointLocation, 80.0f, 80.0f, JunctionPointIcon->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
		PDI->SetHitProxy(NULL);
		
		//End Point
		PDI->SetHitProxy(new HJunctionPointHitProxy(Component, i, EJunctionCommandAction::JUNCTIONENDPOINT));
		FVector EndLocation = FMath::Lerp(DrawPointLocation, JunctionLocation + JunctionSurfaceComponent->GetJunctionCenterLocation(), JunctionPoints[i].ULength);
		PDI->DrawSprite(EndLocation, 50.0f, 50.0f, JunctionEndIcon->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
		PDI->SetHitProxy(NULL);

		//Draw Lane UI
		DrawLaneUI(JunctionPoints[i], JunctionPoints[i].LeftLanes, JunctionPoints[i].RightLanes, LaneEndLocation, PDI, Component, i);

	}

}

bool JunctionVisualiser::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if (VisProxy)
	{
		if (VisProxy && VisProxy->IsA(HJunctionPointHitProxy::StaticGetType()))
		{
			HJunctionPointHitProxy* Proxy = (HJunctionPointHitProxy*)VisProxy;

			if (Proxy->CommandAction == EJunctionCommandAction::JUNCTIONPOINT)
			{
				UE_LOG(LogTemp, Log, TEXT("Vis Proxy was clicked with %i, %i"), Proxy->TargetIndex, Proxy->CommandAction);
				CurrentlySelectedIndex = Proxy->TargetIndex;
				CurrentlySelectedCommandAction = Proxy->CommandAction;

				return true;
			}

			if (Proxy->CommandAction == EJunctionCommandAction::JUNCTIONENDPOINT)
			{
				UE_LOG(LogTemp, Log, TEXT("Vis Proxy was clicked with %i, %i"), Proxy->TargetIndex, Proxy->CommandAction);
				CurrentlySelectedIndex = Proxy->TargetIndex;
				CurrentlySelectedCommandAction = Proxy->CommandAction;

				return true;
			}
		}

		if (VisProxy && VisProxy->IsA(HJunctionLaneWidth::StaticGetType()))
		{
			HJunctionLaneWidth* Proxy = (HJunctionLaneWidth*)VisProxy;

			if (Proxy->CommandAction == EJunctionCommandAction::LANEWIDTH && Proxy->LaneIndex != INDEX_NONE)
			{
				UE_LOG(LogTemp, Log, TEXT("Vis Proxy was clicked with %i, %i, %i"), Proxy->TargetIndex, Proxy->CommandAction, Proxy->LaneIndex);
				CurrentlySelectedIndex = Proxy->TargetIndex;
				CurrentlySelectedCommandAction = Proxy->CommandAction;
				CurrentlySelectedLaneIndex = Proxy->LaneIndex;
				LeftRightSwitch = Proxy->LeftRightSwitch;

				return true;
			}
		}

		if (VisProxy && VisProxy->IsA(HJunctionLaneTurningType::StaticGetType()))
		{
			HJunctionLaneTurningType* Proxy = (HJunctionLaneTurningType*)VisProxy;

			if (Proxy->CommandAction == EJunctionCommandAction::LANEDRIVINGTYPE && Proxy->LaneIndex != INDEX_NONE)
			{
				UE_LOG(LogTemp, Log, TEXT("Vis Proxy was clicked with %i, %i, %i"), Proxy->TargetIndex, Proxy->CommandAction, Proxy->LaneIndex);
				CurrentlySelectedIndex = Proxy->TargetIndex;
				CurrentlySelectedCommandAction = Proxy->CommandAction;
				CurrentlySelectedLaneIndex = Proxy->LaneIndex;
				LeftRightSwitch = Proxy->LeftRightSwitch;

				return true;
			}
		}
	}

	return false;
}

bool JunctionVisualiser::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (CurrentlySelectedIndex != INDEX_NONE && CurrentlySelectedCommandAction == EJunctionCommandAction::JUNCTIONPOINT)
	{
		OutLocation = OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].Location;
		return true;
	}

	if (CurrentlySelectedIndex != INDEX_NONE && CurrentlySelectedCommandAction == EJunctionCommandAction::JUNCTIONENDPOINT)
	{
		OutLocation = OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].EndLocation;
		return true;
	}

	if (CurrentlySelectedCommandAction == EJunctionCommandAction::LANEWIDTH)
	{
		float LaneWidth = OwnedJunctionSurface->GetAccumilatedLaneWidth(OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex], LeftRightSwitch, CurrentlySelectedLaneIndex);
		//UE_LOG(LogTemp, Log, TEXT("Accum Width %f"), LaneWidth);
		OutLocation = OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].EndLocation + (OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].RightVector * LaneWidth);
		return true;
	}

	return false;
}

bool JunctionVisualiser::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const
{
	bool bHandled = false;

	if (CurrentlySelectedIndex != INDEX_NONE && OwnedJunctionSurface != nullptr)
	{
		if (CurrentlySelectedCommandAction == EJunctionCommandAction::JUNCTIONPOINT)
		{
			OutMatrix = FMatrix::Identity;
			bHandled = true;
		}

		if (CurrentlySelectedCommandAction == EJunctionCommandAction::JUNCTIONENDPOINT)
		{
			FVector GetForwardVector = OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].ForwardVector;

			OutMatrix = FRotationMatrix::Make(GetForwardVector.Rotation().Quaternion()); 
			bHandled = true;
		}

		if (CurrentlySelectedCommandAction == EJunctionCommandAction::LANEWIDTH)
		{
			FVector GetForwardVector = OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].RightVector;

			OutMatrix = FRotationMatrix::Make(GetForwardVector.Rotation().Quaternion());
			bHandled = true;
		}
	}

	return bHandled;
}

bool JunctionVisualiser::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	bool bHandled = false;

	if (OwnedJunctionSurface)
	{
		FVector TransformedDrag = OwnedJunctionSurface->GetActorTransform().InverseTransformVector(DeltaTranslate);

		if (CurrentlySelectedIndex != INDEX_NONE && CurrentlySelectedCommandAction == EJunctionCommandAction::JUNCTIONPOINT)
		{
			bHandled = true;
			OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].Location += TransformedDrag;
			OwnedJunctionSurface->RebuildJunctionGeometry();
		}

		if (CurrentlySelectedIndex != INDEX_NONE && CurrentlySelectedCommandAction == EJunctionCommandAction::JUNCTIONENDPOINT)
		{
			bHandled = true;
			float ForwardDelta = FVector::DotProduct(DeltaTranslate, OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].ForwardVector);
			
			//UValue to WorldSpace Distance to Match our Widget
			float Distance = FVector::Distance(OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].Location, OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].EndLocation);
			float NormalizedDistance = ForwardDelta / Distance;

			UE_LOG(LogTemp, Log, TEXT("ForwardDelta %f"), NormalizedDistance);
			OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].ULength = FMath::Clamp(OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].ULength - (NormalizedDistance * 0.05f), 0.05f, 0.95f);
			OwnedJunctionSurface->RebuildJunctionGeometry();
		}
	}

	return bHandled;
}

TSharedPtr<SWidget> JunctionVisualiser::GenerateContextMenu() const
{
	//FMenuBuilder MenuBuilder(true, )
	FMenuBuilder MenuBuilder(true, FJunctionVisualiserActions);
	{
		MenuBuilder.BeginSection("Insert Actions");
		{
		}

		TSharedPtr<FUICommandInfo> ChangeLaneDrivingStatus = nullptr;

		if (FJunctionVisualiserCommands::Get().IsRegistered())
		{
			ChangeLaneDrivingStatus = FJunctionVisualiserCommands::Get().UpdateLaneDrivingType;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Context menu error. Get help."));
		}

		//Check commands
		if (ChangeLaneDrivingStatus != nullptr)
		{
			if (CurrentlySelectedCommandAction == EJunctionCommandAction::LANEWIDTH)
			{
				MenuBuilder.AddSeparator(FName("Driving Type"));
				MenuBuilder.AddSeparator();
				MenuBuilder.AddSubMenu(FText::FromString("Change Lane Driving Type"),
					FText::FromString("Change Lane Driving Type"),
					FNewMenuDelegate::CreateRaw(this, &JunctionVisualiser::BuildLaneDrivingTypeMenu));
				MenuBuilder.AddSeparator(FName("What"));

				MenuBuilder.AddSeparator(FName("Turnung Rule"));
				MenuBuilder.AddSeparator();
				MenuBuilder.AddSubMenu(FText::FromString("Change Lane Turning Rule"),
					FText::FromString("Change Lane Turning Rule"),
					FNewMenuDelegate::CreateRaw(this, &JunctionVisualiser::BuildLaneTurningMenu));
				MenuBuilder.AddSeparator(FName("What"));

				MenuBuilder.AddMenuEntry(
					FText::FromString("Insert Point at this Location"),
					FText::FromString("Insert Point at this Location"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &JunctionVisualiser::RebuildJunctionLanes))
				);
			};
		}
	}

	TSharedPtr<SWidget> MenuWidget = MenuBuilder.MakeWidget();
	return MenuWidget;

}



void JunctionVisualiser::DrawLaneUI(FJunctionPoint InJunctionPoint, TArray<FJunctionLaneData> InLeftLanes, TArray<FJunctionLaneData> InRightLanes, FVector InEndLocation, FPrimitiveDrawInterface* PDI, const UActorComponent* Component, int InLaneIndex)
{
	float AccumilatedWidth = 175.0f;
	int LaneIndex = 0;

	for (FJunctionLaneData LeftLane : InLeftLanes)
	{
		AccumilatedWidth += (LeftLane.LaneWidth);

		FVector LaneIconLocation = InEndLocation + (InJunctionPoint.RightVector * AccumilatedWidth) + FVector(0.0f, 0.0f, 50.0f);

		PDI->SetHitProxy(new HJunctionLaneWidth(Component, InLaneIndex, EJunctionCommandAction::LANEWIDTH, LaneIndex, 0));
		PDI->DrawSprite(LaneIconLocation, 25.0f, 25.0f, JunctionEndIcon->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
		PDI->SetHitProxy(NULL);

		PDI->SetHitProxy(new HJunctionLaneWidth(Component, InLaneIndex, EJunctionCommandAction::LANEWIDTH, LaneIndex, 0));
		PDI->DrawSprite(LaneIconLocation, 25.0f, 25.0f, JunctionEndIcon->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
		PDI->SetHitProxy(NULL);

		//Draw Lane Direction
		//Draw Lane Type
		DrawLaneOptionsUI(InJunctionPoint, LeftLane, LaneIconLocation, PDI, Component, InLaneIndex, LaneIndex, 0);

		LaneIndex += 1;
	}

	LaneIndex = 0;
	AccumilatedWidth = -175.0f;

	for (FJunctionLaneData RightLane : InRightLanes)
	{
		AccumilatedWidth += RightLane.LaneWidth;

		FVector LaneIconLocation = (InEndLocation + (InJunctionPoint.RightVector * AccumilatedWidth)) + FVector(0.0f,0.0f,50.0f);

		PDI->SetHitProxy(new HJunctionLaneWidth(Component, InLaneIndex, EJunctionCommandAction::LANEWIDTH, LaneIndex, 1));
		PDI->DrawSprite(LaneIconLocation, 25.0f, 25.0f, JunctionEndIcon->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
		PDI->SetHitProxy(NULL);

		DrawLaneOptionsUI(InJunctionPoint, RightLane, LaneIconLocation, PDI, Component, InLaneIndex, LaneIndex, 1);

		LaneIndex += 1;
	}

}

void JunctionVisualiser::DrawLaneOptionsUI(FJunctionPoint InJunctionPoint, FJunctionLaneData InLane, FVector InLocation, FPrimitiveDrawInterface* PDI, const UActorComponent* Component, int InJunctionIndex, int InLaneIndex, int InLeftRightSwitch)
{

	ELaneDrivingType LaneType = InLane.RoadType;
	ELaneTurningOptions LaneTurningRule = InLane.TurningRule;
	
	FVector RoadIconLocation = InLocation + FVector(0, 0, 200.0f);
	FVector RoadTurningRuleLocation = InLocation + FVector(0, 0, 100.0f);
	
	switch (LaneType)
	{
	case ELaneDrivingType::NONE:
		PDI->SetHitProxy(new HJunctionLaneTurningType(Component, InJunctionIndex, EJunctionCommandAction::LANEDRIVINGTYPE, InLaneIndex, InLeftRightSwitch));
		PDI->DrawSprite(RoadIconLocation, 30.0f, 30.0f, LaneTypeIconNone->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
		PDI->SetHitProxy(NULL);
		break;
	case ELaneDrivingType::DRIVING:
		PDI->SetHitProxy(new HJunctionLaneTurningType(Component, InJunctionIndex, EJunctionCommandAction::LANEDRIVINGTYPE, InLaneIndex, InLeftRightSwitch));
		PDI->DrawSprite(RoadIconLocation, 30.0f, 30.0f, LaneTypeIconDriving->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
		PDI->SetHitProxy(NULL);
		break;
	case ELaneDrivingType::SHOULDER:
		PDI->SetHitProxy(new HJunctionLaneTurningType(Component, InJunctionIndex, EJunctionCommandAction::LANEDRIVINGTYPE, InLaneIndex, InLeftRightSwitch));
		PDI->DrawSprite(RoadIconLocation, 30.0f, 30.0f, LaneTypeIconShoulder->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
		PDI->SetHitProxy(NULL);
		break;
	case ELaneDrivingType::BICYCLE:
		PDI->SetHitProxy(new HJunctionLaneTurningType(Component, InJunctionIndex, EJunctionCommandAction::LANEDRIVINGTYPE, InLaneIndex, InLeftRightSwitch));
		PDI->DrawSprite(RoadIconLocation, 30.0f, 30.0f, LaneTypeIconBicycle->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
		PDI->SetHitProxy(NULL);
		break;
	default:
		break;
	}

	if (InLeftRightSwitch == 1)
	{
		return;
	}

	//InLane.LaneType == ELaneDrivingType::NONE
	switch (LaneTurningRule)
	{
		case ELaneTurningOptions::LEFT:
			PDI->DrawSprite(RoadTurningRuleLocation, 50.0f, 50.0f, LaneTurningLeft->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
			break;
		case ELaneTurningOptions::LEFTFORWARD:
			PDI->DrawSprite(RoadTurningRuleLocation, 50.0f, 50.0f, LaneTurningLeftForward->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
			break;
		case ELaneTurningOptions::FORWARD:
			PDI->DrawSprite(RoadTurningRuleLocation, 50.0f, 50.0f, LaneTurningForward->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
			break;
		case ELaneTurningOptions::FORWARDRIGHT:
			PDI->DrawSprite(RoadTurningRuleLocation, 50.0f, 50.0f, LaneTurningForwardRight->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
			break;
		case ELaneTurningOptions::RIGHT:
			PDI->DrawSprite(RoadTurningRuleLocation, 50.0f, 50.0f, LaneTurningRight->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
			break;
		case ELaneTurningOptions::ALL:
			PDI->DrawSprite(RoadTurningRuleLocation, 50.0f, 50.0f, LaneTurningAll->GetResource(), FLinearColor::White, SDPG_Foreground, 0, 64, 0, 64, 1, 0.0f);
			break;
	}
	
}

void JunctionVisualiser::UpdateLaneDrivingType(ELaneDrivingType InDrivingType) const
{

	if (CurrentlySelectedLaneIndex != INDEX_NONE && CurrentlySelectedIndex != INDEX_NONE && OwnedJunctionSurface != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("HEY OUR DRIVING MENU THING WORKED"));
		if (LeftRightSwitch == 0)
		{
			OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].LeftLanes[CurrentlySelectedLaneIndex].RoadType = InDrivingType;
		}
		else
		{
			OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].RightLanes[CurrentlySelectedLaneIndex].RoadType = InDrivingType;
		}

		OwnedJunctionSurface->UpdateComponentVisulizer();
	}
}

void JunctionVisualiser::UpdateLaneTurningRule(ELaneTurningOptions InTurningRule) const
{

	if (CurrentlySelectedLaneIndex != INDEX_NONE && CurrentlySelectedIndex != INDEX_NONE && OwnedJunctionSurface != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("HEY OUR DRIVING MENU THING WORKED"));
		OwnedJunctionSurface->JunctionPoints[CurrentlySelectedIndex].LeftLanes[CurrentlySelectedLaneIndex].TurningRule = InTurningRule;

		OwnedJunctionSurface->UpdateComponentVisulizer();
	}
}

void JunctionVisualiser::RebuildJunctionLanes() const
{
	if (OwnedJunctionSurface)
	{
		OwnedJunctionSurface->BuildAndUpdateLaneSplines();
	}

}

void JunctionVisualiser::BuildLaneDrivingTypeMenu(FMenuBuilder& MenuBuilder) const
{
	for (ELaneDrivingType DrivingType : TEnumRange<ELaneDrivingType>())
	{
		FString EnumName = StaticEnum<ELaneDrivingType>()->GetNameStringByValue(static_cast<int64>(DrivingType));

		MenuBuilder.AddMenuEntry(
			FText::FromString(*EnumName),
			FText::FromString("Menu Entry 2 Tooltip"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &JunctionVisualiser::UpdateLaneDrivingType, DrivingType))
		);
	}
}

void JunctionVisualiser::BuildLaneTurningMenu(FMenuBuilder& MenuBuilder) const
{
	for (ELaneTurningOptions RuleType : TEnumRange<ELaneTurningOptions>())
	{
		FString EnumName = StaticEnum<ELaneTurningOptions>()->GetNameStringByValue(static_cast<int64>(RuleType));

		MenuBuilder.AddMenuEntry(
			FText::FromString(*EnumName),
			FText::FromString("Menu Entry 2 Tooltip"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &JunctionVisualiser::UpdateLaneTurningRule, RuleType))
		);
	}
}


#undef LOCTEXT_NAMESPACE