// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaneSpline.h"
#include "AIVehicleDynamicsUtilities.h"
#include "BaseTrafficVehicle.generated.h"

class UTrafficWheelComponent;
class UVehicleDynamicsDataAsset;

//CAN DEP THIS
USTRUCT()
struct FWheelData
{

	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTrafficWheelComponent> WheelComponent = nullptr;
	
	UPROPERTY(EditAnywhere)
	float CurrentCompression = 0.0f;

	UPROPERTY(EditAnywhere)
	float PreviousCompression = 0.0f;
	
	UPROPERTY(EditAnywhere)
	float Velocity = 0.0f;

	UPROPERTY(EditAnywhere)
	float RestLength = 200.0f;

	UPROPERTY(EditAnywhere)
	float MaxLength = 250.0f;

	UPROPERTY(EditAnywhere)
	float SuspensionStiffness = 200000.0f;

	UPROPERTY(EditAnywhere)
	float SuspensionDamping = 4000.0f;

};


UCLASS()
class DRIVING_API ABaseTrafficVehicle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseTrafficVehicle();

	void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	bool BuildVehicleData();

	UFUNCTION()
	void CreateWheelMeshes();

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

	//Physics / Vehicle Locomotion - can move this to utilities later
	UFUNCTION()
	void ResolveVehiclePhysics(float InDeltaTime, FVector InSplineLocation, FRotator InSplineRotation);
	
	UFUNCTION()
	FVector CalculateCOM();

	UFUNCTION()
	void UpdateWheelRotation(int InWheelID, float InDeltaTime, FVector InVehicleLocation);

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UVehicleDynamicsDataAsset> VehicleDynamicsDataAsset;

	UPROPERTY()
	FVehicleDynamicsData VehicleDynamicsStruct;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> VehicleBodyMesh;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> WheelMeshes;
	UPROPERTY(VisibleAnywhere)
	float WheelRotationDelta = 0.0f;
	UPROPERTY(VisibleAnywhere)
	float SteerAngle = 0.0f;
	UPROPERTY(VisibleAnywhere)
	float PreCalculatedWheelCircumference = 100.0f;

	UPROPERTY(EditAnywhere)
	TArray<FWheelData> WheelData; //CAN DEP

	UPROPERTY(EditAnywhere)
	TArray<FVector> WheelVectors; //DUNNO
	UPROPERTY(EditAnywhere)
	TArray<FVector> WheelRaycastPositions; //DUNNO

	//Car Current Values -- What is the car doing on this frame?
	UPROPERTY(VisibleAnywhere)
	float VehicleSpeed = 0.0f; //Current Speed
	UPROPERTY(EditAnywhere)
	FVector VehicleWorldLocation; //CAN DEP
	UPROPERTY(EditAnywhere)
	FRotator VehicleWorldRotation; //Do we need this?
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

	//Vehicle Properties
	UPROPERTY(EditAnywhere)
	float VehicleMaxSpeed = 60.0f; //Max Speed CAN DEP


	//Vehicle Init properties -- Set by Traffic Manager
	UPROPERTY(EditAnywhere)
	float SpawnPositionAlongSplineLength = 0.0;
	UPROPERTY(EditAnywhere)
	int VehicleID = 0;

	//Vehicle Traffic Manager Properties - Updating Spawning etc
	UPROPERTY(VisibleAnywhere)
	bool MarkedForRespawn = false;

	//V2 Using Custom Physics to do car animation etc <-- merge all this code when done but 
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool UseV2Physics = false;

	//CLEAN THIS UP - AS ITS MOVED INTO DATAASSET
	UPROPERTY(EditAnywhere)
	FVector Gravity = FVector(0.0f,0.0f,-980.0f); 
	UPROPERTY(EditAnywhere)
	float VehicleMass = 2000.0f; //kg?
	UPROPERTY(EditAnywhere)
	float VerticalVelocity = 0.0f;
	UPROPERTY(EditAnywhere)
	float CarBodyHeightOffset = 0.0f;
	UPROPERTY(EditAnywhere)
	float SuspensionOffset = 50.0f; //Move suspension anchor Up or Down
	UPROPERTY(EditAnywhere)
	float MaxSuspensionLength = 80.0f; //Suspension Length
	UPROPERTY(EditAnywhere)
	float SuspensionRestLength = 80.0f;
	UPROPERTY(EditAnywhere)
	float SuspensionSpringStiffness = 20000.0f;
	UPROPERTY(EditAnywhere)
	float SuspensionDampingCoefficient = 1000.0f;
	UPROPERTY(EditAnywhere)
	TArray<float> SuspensionPrevDistance = { 0.0f,0.0f,0.0f,0.0f };
	UPROPERTY(EditAnywhere)
	float WheelRadius = 0.0f; //Suspension Length
	UPROPERTY(EditAnywhere)
	FVector VehicleVelocity = FVector::ZeroVector;
	UPROPERTY(EditAnywhere)
	FVector VehicleAngularVelocity = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere)
	FVector AccumilatedForce = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere)
	FVector AccumilatedTorque = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere)
	FVector VehicleCOM = FVector::ZeroVector;
	UPROPERTY(EditAnywhere)
	FVector InertialTensor = FVector(10000.0f, 10000.0f, 10000.0f); // tweak manually
	UPROPERTY(EditAnywhere)
	float IntertialTensorScale = 1.0f;
	FQuat ActorRotation;
	UPROPERTY()
	FVector PreviousLocation; //Store the previous Frame Delta so we can update our speed
	UPROPERTY()
	float CurrentSpeed;

	//Physics Debug
	UPROPERTY(EditAnywhere)
	FVector CarBodyBoxLocation = FVector(0, 0, 0);
	UPROPERTY(EditAnywhere)
	FVector CarBodyBoxExtents = FVector(150, 60, 75);

	UFUNCTION() //DEP THIS
	void DrivingVersionTwo(float DeltaTime);


};
