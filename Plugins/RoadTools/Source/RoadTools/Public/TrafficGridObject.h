// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaneSpline.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TrafficGridObject.generated.h"


USTRUCT()
struct FConnectedRoads
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	ALaneSpline* ReferencedLaneSpline;
	UPROPERTY(VisibleAnywhere)
	TArray<int> PointID;

	struct FConnectedRoads()
	{

	}
};


UCLASS()
class ROADTOOLS_API ATrafficGridObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrafficGridObject();

	
	//UFUNCTION()
	//void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//UFUNCTION()
	//void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:	

	UBoxComponent* CollisionBounds = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	int CellID = 0;
	UPROPERTY(EditAnywhere)
	FVector Location;
	UPROPERTY(EditAnywhere)
	FVector CellSize = FVector(500,500,500);
	UPROPERTY(EditAnywhere)
	TArray<ALaneSpline*> AssociatedLanes;
	UPROPERTY(EditAnywhere)
	TArray<ATrafficGridObject*> NeighbourCells;
	//UPROPERTY(EditAnywhere)
	//FVector BoxExtent;
	UPROPERTY(EditAnywhere)
	TArray<FConnectedRoads> ConnectedRoads;
	UPROPERTY(EditAnywhere)
	bool isActive = false;


	//Editor Controls
	UPROPERTY(EditAnywhere)
	bool UpdateLanes = false;
	UPROPERTY(EditAnywhere)
	bool DebugDrawLanes = false;
	UPROPERTY(EditAnywhere)
	bool DebugDrawCells = false;

	//Debug Data
	TArray<FVector> DrawPointsPositions;

	//Functions
	UFUNCTION()
	void GetAllIntersectingLanes();
	UFUNCTION()
	void SetBoxExtent(FVector Extent);

	//Debug Functions
	UFUNCTION()
	void DebugDrawConnectedLanes();
	UFUNCTION()
	void DebugDrawNeighbourCells();


	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
