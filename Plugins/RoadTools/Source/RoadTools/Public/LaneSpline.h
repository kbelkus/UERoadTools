// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SplineComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaneSpline.generated.h"



USTRUCT()
struct FLaneConnections
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ALaneSpline* LaneReference;
	UPROPERTY(EditAnywhere)
	int ConnectionPosition;

	FLaneConnections()
	{
		//SplineID = int(0);
	}
};


UCLASS()
class ROADTOOLS_API ALaneSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaneSpline();


	UFUNCTION()
	void DebugDrawLaneDirection();
	void DebugDrawLaneConnections();
	void DebugDrawLaneStatus();
	UFUNCTION()
	void ReverseSpline();
	UFUNCTION()
	void UpdateSplineStatus();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Override Construction SCript
	virtual void OnConstruction(const FTransform& Transform) override;

	TArray<FColor> DebugColours = { FColor::Red, FColor::Blue, FColor::Green, FColor::Yellow, FColor::Cyan };

	TArray<FColor> LaneStatusColors = { FColor::Red, FColor::Green, FColor::Blue };


public:	

	//EDITOR PROPERTIES
	UPROPERTY(EditAnywhere)
	bool DrawDebug;
	UPROPERTY(EditAnywhere)
	bool FlipLaneDirection;
	UPROPERTY(EditAnywhere)
	bool isJunctionLane;
	UPROPERTY(EditAnywhere)
	USplineComponent* LaneSpline;
	

	USplineComponent* ReturnSpline() const { return LaneSpline; }

	//SET BY EDITOR QUERY BY VEHICLE
	UPROPERTY(EditAnywhere)
	TArray<FLaneConnections> LaneConnections;
	UPROPERTY(EditAnywhere)
	int LaneDirection = 0;
	
	//SET BY TRAFFIC LIGHT
	UPROPERTY(EditAnywhere)
	int LaneStatus = 1; //0 = stop 1 = go 2 = ??


	FVector LaneStartPosition;


	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
