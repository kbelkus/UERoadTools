// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaneSpline.h"
#include "BaseTrafficVehicle.generated.h"

UCLASS()
class DRIVING_API ABaseTrafficVehicle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseTrafficVehicle();

	UFUNCTION()
	void SetSpawnLocation();

	//Vehicle Logic
	UFUNCTION()
	void Driving(float dt);
	UFUNCTION()
	void VehicleTargetUpdate(float dt);
	UFUNCTION()
	FVector ResolveVehicleLocation(FVector SplineLocation, FRotator Rotation);
	

	//Car Target
	UFUNCTION()
	void GetNextLane();
	UFUNCTION()
	void CollisionCheck();
	UFUNCTION()
	void DebugDrawTarget();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	//Random Range
	UFUNCTION()
	int32 GetRandomLaneID(int32 Min, int32 Max)
	{
		FMath::RandInit(FDateTime::UtcNow().GetTicks() + GetUniqueID());

		return FMath::RandRange(Min, Max);
	}
		

	//Car Tweakables
	UPROPERTY(EditAnywhere)
		TArray<FVector> WheelOffsets = 
	{ 
		FVector(150.0f,75.0f,0.0f),
		FVector(150.0f,-75.0f,0.0f),
		FVector(-100.0f,75.0f,0.0f),
		FVector(-100.0f,-75.0f,0.0f)
	};

	UPROPERTY(EditAnywhere)
	TArray<FVector> WheelVectors;
	UPROPERTY(EditAnywhere)
	TArray<FVector> WheelRaycastPositions;

	//Car Current Values -- What is the car doing on this frame?
	UPROPERTY(EditAnywhere)
	float VehicleSpeed = 0.0f; //Current Speed
	UPROPERTY(EditAnywhere)
	FVector VehicleWorldLocation;
	UPROPERTY(EditAnywhere)
	FRotator VehicleWorldRotation;
	UPROPERTY(EditAnywhere)
	ALaneSpline* CurrentLane;
	UPROPERTY()
	USplineComponent* CurrentSpline; //Actual spline return this for speed
	UPROPERTY()
	float CurrentLaneLength;
	UPROPERTY(EditAnywhere)
	float PositionAlongSplineLocal = 0.0;
	UPROPERTY(EditAnywhere)
	float PositionAlongSplineLength = 0.0;
	UPROPERTY(EditAnywhere)
	bool isValidVehicle = false;
	UPROPERTY(EditAnywhere)
	int LaneDrivingDirection;
	UPROPERTY(EditAnywhere)
	bool IgnoreSignal = false;

	UPROPERTY(EditAnywhere)
	FString CollidedObject;
	FVector CollidedObjectLocation;

	//Target Properties
	int LaneStatus = 1;
	int NextLaneID;
	ALaneSpline* NextLane;
	USplineComponent* NextSpline;
	float NextLaneLength;
	float NextPositionAlongSplineLength;
	int NextLaneDrivingDirection;
	UPROPERTY(EditAnywhere)
	int NextLaneStatus = 1;



	FVector RayHeightOffset = FVector(0, 0, 100);

	//Vehicle Target Properties - Intended Direction, Collision Detection etc
	FVector ForwardVector;
	FVector RayStartPosition;
	UPROPERTY(EditAnywhere)
	bool detectedObstacle = false;
	UPROPERTY(EditAnywhere)
	float DistanceToObstacle;
	FCollisionQueryParams CollisionParms;
	int CollidedLaneDrivingDirection;
	FVector TargetLocation;
	UPROPERTY(EditAnywhere)
	bool FoundNextLane = false; //Check so that, we dont keep getting a new lane when we are sat at traffic light

	//Vehicle Properties - Stuff thatcont
	UPROPERTY(EditAnywhere)
	float VehicleMaxSpeed = 60.0f; //Max Speed
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* VehicleMesh;
	UPROPERTY(EditAnywhere)
	UStaticMesh* VehicleBodyMesh;

	//Vehicle Init properties -- Set by Traffic Manager
	UPROPERTY(EditAnywhere)
	float SpawnPositionAlongSplineLength = 0.0;
	UPROPERTY(EditAnywhere)
	int VehicleID = 0;

	//Vehicle Traffic Manager Properties - Updating Spawning etc
	UPROPERTY(VisibleAnywhere)
	bool MarkedForRespawn = false;



};
