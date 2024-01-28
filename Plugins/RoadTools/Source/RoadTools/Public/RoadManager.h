// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadSurface.h"
#include "JunctionSurface.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "RoadManager.generated.h"



USTRUCT()
struct FRoadIntersections
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<ARoadSurface*> RoadReference;
	UPROPERTY(EditAnywhere)
	TArray<int> RoadID;

	UPROPERTY(EditAnywhere)
	FVector IntersectPosition;

	UPROPERTY(EditAnywhere)
	TArray<int> PointID; //This should be in the same order as the Road Reference

	FRoadIntersections()
	{
		//SplineID = int(0);
	}
};


USTRUCT()
struct FCurvePoints
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int RoadReference;

	UPROPERTY(EditAnywhere)
	FVector PointPosition;

	UPROPERTY(EditAnywhere)
	int PointID;  //This should be in the same order as the SplineReference

	FCurvePoints()
	{
		//SplineID = int(0);
	}
};


//Generic Point Struct to copy and store Point Position, Normal, Right Vec etc
USTRUCT()
struct FRoadPoint
{
	GENERATED_BODY()

	UPROPERTY()
	FVector WorldSpacePosition;
	FVector WorldSpaceNormal;
	FVector WorldSpaceRightVector;
	ARoadSurface* ConnectedRoad;

	FRoadPoint()
	{

	}
};

// MOVE TO OWN CLASS
USTRUCT()
struct FGenericRoadPoint
{
	GENERATED_BODY()

	UPROPERTY()
	FVector WorldSpacePosition;
	FVector WorldSpaceNormal;
	FVector WorldSpaceRightVector;
	ARoadSurface* ConnectedRoad;

	FGenericRoadPoint()
	{

	}
};

USTRUCT()
struct FPointOnRoadCurveJunction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ARoadSurface* RoadSurface;
	UPROPERTY(EditAnywhere)
	int CurvePointID;

	FPointOnRoadCurveJunction()
	{

	}
};




//Set This to each RoadSurface to cut the road at junction points
USTRUCT()
struct FRoadSurfaceIntersectPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<float> DistanceAlongCurve;

	FRoadSurfaceIntersectPoint()
	{

	}
};



//This struct holds ALL the information we need when we create a junction
USTRUCT()
struct FJunctionStructure
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	TArray<FRoadPoint> RoadPoints;
	UPROPERTY(EditAnywhere)
	int JunctionID;
	UPROPERTY(EditAnywhere)
	TArray<ARoadSurface*> ConnectedRoad;
	UPROPERTY(EditAnywhere)
	FVector WorldPosition;
	

	FJunctionStructure()
	{
		//SplineID = int(0);
	}
};



UCLASS()
class ROADTOOLS_API ARoadManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoadManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UFUNCTION()
	void GatherRoads(UWorld* World);
	void CreateJunctions(UWorld* World);
	void GatherJunctions(UWorld* World);
	void SolveIntersections();
	void SolveIntersectionKeyPoints();
	void CreateJunctionObject(UWorld* World);
	void GetAllCurvePoints();
	void UpdateRoadSurfaces();

	UFUNCTION()
	void DebugDrawIntersections();


	//Override Construction SCript
	virtual void OnConstruction(const FTransform& Transform) override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	TArray<ARoadSurface*> RoadCollection;
	UPROPERTY(EditAnywhere)
	TArray<AJunctionSurface*> JunctionCollection;
	UPROPERTY(EditAnywhere)
	bool UpdateRoads;
	UPROPERTY(EditAnywhere)
	TArray<FRoadIntersections> Intersections;
	UPROPERTY(EditAnywhere)
	TArray<FCurvePoints> CurvePoints;
	UPROPERTY(EditAnywhere)
	TMap<FVector, FRoadIntersections> JunctionData; //Test Temp
	UPROPERTY(EditAnywhere)
	TArray<FJunctionStructure> JunctionConstructionData;
	UPROPERTY(EditAnywhere)
	TMap<ARoadSurface*, FRoadSurfaceIntersectPoint> RoadIntersections;
	UPROPERTY(EditAnywhere)
	TMap<ARoadSurface*, FPointOnRoadCurveJunction> RoadPointsJunctions;
	UPROPERTY(EditAnywhere)
	TArray<FPointOnRoadCurveJunction> RoadJunctionPointIDs;
	

};
