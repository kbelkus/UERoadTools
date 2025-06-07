// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SidewalkSurface.generated.h"

class USplineComponent;
class ARoadSurface;
class AJunctionSurface;
class UProceduralMeshComponent;


USTRUCT() //MOVE THIS TO MESH UTILITIES
struct FVertexBufferSimple
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVector> Location;

	UPROPERTY()
	TArray<FVector> Normal;

	UPROPERTY()
	TArray<FVector2D> UV;

	FVertexBufferSimple()
	{

	}
};


UENUM(BlueprintType)
enum class ESideWalkGenerationSide : uint8
{
	LEFT = 0 UMETA(DisplayName = "Left"),
	RIGHT = 1  UMETA(DisplayName = "Right"),
	BOTH = 2     UMETA(DisplayName = "Both"),
};

UENUM(BlueprintType)
enum class ESideWalkGenerationType : uint8
{
	NONE = 0 UMETA(DisplayName = "Invalid"),
	ROAD = 1 UMETA(DisplayName = "Road"),
	JUNCTION = 2  UMETA(DisplayName = "Junction"),
};


UCLASS()
class ROADTOOLS_API ASidewalkSurface : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASidewalkSurface();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> PrimaryEdgeSpline;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> SecondaryEdgeSpline;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> SidewalkPrimarySurface;


	UFUNCTION(CallInEditor)
	void RebuildSideWalkSurface();

	UFUNCTION()
	bool UpdatePrimaryEdgeSpline();

	UFUNCTION()
	bool GetRoadSurfaceEdge();

	UFUNCTION()
	void GetJunctionEdge();

	UFUNCTION(CallInEditor)
	void InitialiseSideWalkProperties();

	UFUNCTION()
	void RebuildSideWalkGeometry();

	//Mesh Functions Split this out later
	UFUNCTION()
	FVertexBufferSimple BuildMeshGridFromSpline(USplineComponent* InSplineComponent, int InXResolution, int InYResolution, float OffsetY, bool ToggleLeftRight, float InWidth);

	UFUNCTION()
	TArray<int> BuildTriangleIndices(int InXResolution, int InYResolution, bool ReverseWindingOrder);

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> DebugDrawPositions;



protected:

	virtual void OnConstruction(const FTransform& Transform) override;

public:	

	UPROPERTY(EditAnywhere)
	ESideWalkGenerationSide SideWalkGenerationSide = ESideWalkGenerationSide::LEFT;
	 
	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> ConnectedSurface;

	UPROPERTY(EditAnywhere)
	float SideWalkWidth = 300.0f; //Make Array Later

	UPROPERTY(EditAnywhere)
	int WidthResolution = 2;

	UPROPERTY(EditAnywhere)
	int LengthResolution = 10;

	//Automatic Hidden Properties
	UPROPERTY(VisibleAnywhere)
	ESideWalkGenerationType SideWalkGenerationType;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ARoadSurface> ConnectedRoadSurface;



};
