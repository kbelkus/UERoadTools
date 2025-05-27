// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ComponentVisualizer.h"
#include "HitProxies.h"
#include "CoreMinimal.h"
#include "JunctionSurface.h"
#include "JunctionSurfaceComponent.h"

UENUM(BlueprintType)
enum class EJunctionCommandAction : uint8
{
	NONE			 = 0 UMETA(DisplayName = "None"),
	JUNCTIONPOINT	 = 1 UMETA(DisplayName = "Junction Point"),
	JUNCTIONENDPOINT = 2 UMETA(DisplayName = "Junction End Point"),
	LANEWIDTH		 = 3 UMETA(DisplayName = "Lane Width"),
	LANEADD			 = 4 UMETA(DisplayName = "Lane Add"),
	LANEREMOVE		 = 5 UMETA(DisplayName = "Lane Remove"),
	LANECAMBER		 = 6 UMETA(DisplayName = "Lane Camber"),
	JUNCTIONHEIGHT	 = 7 UMETA(DisplayName = "Junction Height"),
	LANETURNINGTYPE	 = 8 UMETA(DisplayName = "Lane Turning Type"),
	LANEDRIVINGTYPE  = 9 UMETA(DisplayName = "Lane Driving Type"),
};


class ROADTOOLS_API JunctionVisualiser : public FComponentVisualizer
{
public:
	JunctionVisualiser();
	~JunctionVisualiser();

	void OnRegister();

	//Interface Overrides
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	//virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	//virtual bool IsVisualizingArchetype() const override;
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click);
	//virtual void EndEditing() override;
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual bool GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	//virtual void TrackingStarted(FEditorViewportClient* InViewportClient) override;
	//virtual void TrackingStopped(FEditorViewportClient* InViewportClient, bool bInDidMove) override;
	//virtual bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual TSharedPtr<SWidget> GenerateContextMenu() const override;

	//Interface Overrides
	TObjectPtr<AJunctionSurface> OwnedJunctionSurface;

	//UI
	TSoftObjectPtr<UTexture2D> JunctionPointIcon;
	TSoftObjectPtr<UTexture2D> JunctionEndIcon;
	TSoftObjectPtr<UTexture2D> LaneOptionsIcon;
	TSoftObjectPtr<UTexture2D> LaneTrafficIcon;
	TSoftObjectPtr<UTexture2D> LaneTurningIcon;
	TSoftObjectPtr<UTexture2D> LaneTypeIcon;
	
	//Lane Type Icons
	TSoftObjectPtr<UTexture2D> LaneTypeIconNone;
	TSoftObjectPtr<UTexture2D> LaneTypeIconDriving;
	TSoftObjectPtr<UTexture2D> LaneTypeIconBicycle;
	TSoftObjectPtr<UTexture2D> LaneTypeIconShoulder;

	//Turning Icons
	TSoftObjectPtr<UTexture2D> LaneTurningLeft;
	TSoftObjectPtr<UTexture2D> LaneTurningForward;
	TSoftObjectPtr<UTexture2D> LaneTurningLeftForward;
	TSoftObjectPtr<UTexture2D> LaneTurningAll;
	TSoftObjectPtr<UTexture2D> LaneTurningForwardRight;
	TSoftObjectPtr<UTexture2D> LaneTurningRight;


	//FUNCTIONS
	virtual void DrawLaneUI(FJunctionPoint InJunctionPoint, TArray<FJunctionLaneData> InLeftLanes, TArray<FJunctionLaneData> InRightLanes, FVector InEndLocation, FPrimitiveDrawInterface* PDI, const UActorComponent* Component, int InLaneIndex);
	virtual void DrawLaneOptionsUI(FJunctionPoint InJunctionPoint, FJunctionLaneData InLane, FVector InEndLocation, FPrimitiveDrawInterface* PDI, const UActorComponent* Component, int InJunctionIndex, int InLaneIndex, int InLeftRightSwitch);

	//UI Selection //Could move this to its own class with validation?
	int CurrentlySelectedIndex = INDEX_NONE;
	EJunctionCommandAction CurrentlySelectedCommandAction = EJunctionCommandAction::NONE;
	int CurrentlySelectedLaneIndex = INDEX_NONE;
	int LeftRightSwitch = INDEX_NONE;
	

};


