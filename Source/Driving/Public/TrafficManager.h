// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTrafficVehicle.h"
#include "LaneSpline.h"
#include "TrafficGridObject.h"
#include "TrafficManager.generated.h"


USTRUCT()
struct FLanes
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ALaneSpline* Lane;
	UPROPERTY(EditAnywhere)
	FVector LanePosition;

	FLanes()
	{

	}
};


USTRUCT()
struct FTrafficAgents
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ABaseTrafficVehicle* TrafficAgent;
	UPROPERTY(EditAnywhere)
	float DistanceFromPlayer;
	UPROPERTY(EditAnywhere)
	float PreviousDistanceFromPlayer;
	UPROPERTY(EditAnywhere)
	float delta;
	UPROPERTY(EditAnywhere)
	int PositiveDeltaCount = 0;

	FTrafficAgents()
	{

	}
};


USTRUCT()
struct FLaneSplinePoints
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	ALaneSpline* ReferencedLaneSpline;
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> PointLocation;
	UPROPERTY(VisibleAnywhere)
	TArray<int> PointID;

	struct FLaneSplinePoints()
	{

	}
};



UCLASS()
class DRIVING_API ATrafficManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrafficManager();

	UFUNCTION()
	void GetLanes();
	UFUNCTION()
	void InitialiseTrafficAgents();
	UFUNCTION()
	void UpdateTrafficAgents();
	UFUNCTION()
	void UpdateSingleTrafficAgent(FTrafficAgents TrafficAgent);
	UFUNCTION()
	void UpdatePlayerCell();
	UFUNCTION()
	void RespawnTrafficAgents();

	//Traffic Grid
	UFUNCTION()
	void CreateTrafficGrid();
	UFUNCTION()
	void GetAllLaneSplinePoints();
	UFUNCTION()
	void AssignLanesToCells();
	UFUNCTION()
	void AssignNeighbourCells();
	UFUNCTION(CallInEditor)
	void DestroyNeighbourCells();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Override Construction SCript
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	int32 GetRandomInt(int32 Min, int32 Max)
	{
		FMath::RandInit(FDateTime::UtcNow().GetTicks() + GetUniqueID());
		return FMath::RandRange(Min, Max);
	}


public:	

	UPROPERTY(EditAnywhere)
	TArray<ABaseTrafficVehicle*> TrafficLibrary;
	UPROPERTY(EditAnywhere)
	FVector PlayerWorldLocation;
	UPROPERTY(EditAnywhere)
	float MaxVehicleDistance = 10000;
	UPROPERTY(EditAnywhere)
	float MaxLaneSearchDistance = 2000;
	UPROPERTY(EditAnywhere)
	int TrafficAgentsPoolSize = 15;
	UPROPERTY(EditAnywhere)
	TArray<FTrafficAgents> TrafficAgentsPool;
	UPROPERTY(EditAnywhere)
	TArray<FLanes> AvailableRoads;
	UPROPERTY(EditAnywhere)
	TArray<int> TrafficAgentRespawnList;

	UPROPERTY(EditAnywhere)
	TArray<ALaneSpline*> CurrentSpawnableLanes;
	
	//Grid Properties
	UPROPERTY(EditAnywhere)
	bool RebuildGrid = false;
	UPROPERTY(EditAnywhere)
	FVector GridWorldBounds = FVector(5000,5000,5000);
	UPROPERTY(EditAnywhere)
	FVector GridResolution = FVector(10,10,10);
	UPROPERTY(EditAnywhere)
	int GridDivisions = 10.0f;
	UPROPERTY(VisibleAnywhere)
	FVector IndividualCellSize;
	UPROPERTY(VisibleAnywhere)
	FVector CellsOffsetSpace;



	UPROPERTY(VisibleAnywhere)
	TArray<FLaneSplinePoints> RoadPoints;
	UPROPERTY(VisibleAnywhere)
	TArray<ATrafficGridObject*> TrafficCells;
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> CellPositions;
	UPROPERTY()
	int ActiveCellIndex = 0;
	UPROPERTY(VisibleAnywhere)
	TArray<ATrafficGridObject*>ActiveSpawnCells;

	//Spawn Collision Data
	FCollisionQueryParams CollisionParms;


	float ElapsedTimer;


	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
