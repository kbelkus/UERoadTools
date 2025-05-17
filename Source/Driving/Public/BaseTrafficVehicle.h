// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaneSpline.h"
#include "AIVehicleDynamicsUtilities.h"
#include "BaseTrafficVehicle.generated.h"

class UTrafficWheelComponent;
class UVehicleDynamicsDataAsset;

DECLARE_STATS_GROUP(TEXT("Vehicle Dynamics"), STATGROUP_VehicleAI, STATCAT_VehicleAI);


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

UENUM(BlueprintType)
enum class EVehicleStatus : uint8
{
	DRIVING = 0 UMETA(DisplayName = "Driving"),
	STOPPED = 1  UMETA(DisplayName = "Stopped"),
	PARKED = 2     UMETA(DisplayName = "Parked"),
	CRASHED= 3 UMETA(DisplayName = "Crashed"),
};

UENUM(BlueprintType)
enum class EDriverSearchType : uint8
{
	FORWARD = 0 UMETA(DisplayName = "Forward"),
	TARGET = 1  UMETA(DisplayName = "Target"),
};


UCLASS()
class DRIVING_API ABaseTrafficVehicle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseTrafficVehicle();

	//Car Construction Logic
	void OnConstruction(const FTransform& Transform) override;
	UFUNCTION()
	bool BuildVehicleData();
	UFUNCTION()
	void CreateWheelMeshes();
	UFUNCTION()
	void SetSpawnLocation(bool& HasValidSpawn);

	//Vehicle Logic
	UFUNCTION()
	void Driving(float dt);
	UFUNCTION()
	void VehicleTargetUpdate(float dt);
	UFUNCTION()
	FVector ResolveVehicleLocation(FVector SplineLocation, FRotator Rotation);
	
	//Car Driving Logic
	UFUNCTION()
	void GetNextLane();
	UFUNCTION()
	void CollisionCheck();
	UFUNCTION()
	void DebugDrawTarget();

	//Car Dynamics/Physics Logic
	UFUNCTION()
	void ResolveVehiclePhysics(float InDeltaTime, FVector InSplineLocation, FRotator InSplineRotation);
	UFUNCTION()
	FVector CalculateCOM();
	UFUNCTION()
	void UpdateWheelRotation(int InWheelID, float InDeltaTime, FVector InVehicleLocation, float InSpeedDelta);
	UFUNCTION()
	float ApplyBrakingForce(float InNormalizedDistance, float InCurrentSpeed);


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
		
	////Car Tweakables
	//UPROPERTY(EditAnywhere)
	//	TArray<FVector> WheelOffsets = 
	//{ 
	//	FVector(150.0f,75.0f,0.0f),
	//	FVector(150.0f,-75.0f,0.0f),
	//	FVector(-100.0f,75.0f,0.0f),
	//	FVector(-100.0f,-75.0f,0.0f)
	//};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UVehicleDynamicsDataAsset> VehicleDynamicsDataAsset;

	UPROPERTY()
	FVehicleDynamicsData VehicleDynamicsStruct;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> VehicleBodyMesh;

	UPROPERTY(EditAnywhere)
	EVehicleStatus VehicleCurrentStatus = EVehicleStatus::DRIVING;
	UPROPERTY(EditAnywhere)
	EDriverSearchType DriverSearchType = EDriverSearchType::FORWARD;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> WheelMeshes;
	UPROPERTY(VisibleAnywhere)
	float WheelRotationDelta = 0.0f;
	UPROPERTY(VisibleAnywhere)
	float SteerAngle = 0.0f;
	UPROPERTY(VisibleAnywhere)
	float PreCalculatedWheelCircumference = 100.0f;

	//Car Current Values -- What is the car doing on this frame?
	UPROPERTY(VisibleAnywhere)
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
	UPROPERTY(VisibleAnywhere)
	int LaneStatus = 1;
	UPROPERTY(VisibleAnywhere)
	int NextLaneID;
	UPROPERTY(VisibleAnywhere)
	ALaneSpline* NextLane;
	UPROPERTY(VisibleAnywhere)
	USplineComponent* NextSpline;
	UPROPERTY(VisibleAnywhere)
	float NextLaneLength;
	UPROPERTY(VisibleAnywhere)
	float NextPositionAlongSplineLength;
	UPROPERTY(VisibleAnywhere)
	int NextLaneDrivingDirection;
	UPROPERTY(EditAnywhere)
	int NextLaneStatus = 1;

	//Driving RayHeight
	FVector RayHeightOffset = FVector(0, 0, 100); 

	//Vehicle Target Properties - Intended Direction, Collision Detection etc
	FVector ForwardVector;
	FVector RayStartPosition;
	UPROPERTY(EditAnywhere)
	bool detectedObstacle = false;
	UPROPERTY(EditAnywhere)
	float DistanceToObstacle;
	FCollisionQueryParams CollisionParms;
	int CollidedLaneDrivingDirection = -1;
	FVector TargetLocation;
	UPROPERTY(EditAnywhere)
	bool FoundNextLane = false; //Check so that, we dont keep getting a new lane when we are sat at traffic light
	UPROPERTY(EditAnywhere)
	float CollisionDetectionDistance = 1200.0f; //Move this to AI dataasset

	//Vehicle Properties
	UPROPERTY(EditAnywhere)
	float VehicleMaxSpeed = 60.0f; //Move this to vehicle properties

	//Vehicle Init properties -- Set by Traffic Manager
	UPROPERTY(EditAnywhere)
	float SpawnPositionAlongSplineLength = 0.0;
	UPROPERTY(EditAnywhere)
	int VehicleID = 0;

	//Vehicle Traffic Manager Properties - Updating Spawning etc
	UPROPERTY(VisibleAnywhere)
	bool MarkedForRespawn = false;

	//Vehicle Dynamics Data for this actor
	UPROPERTY(EditAnywhere)
	FVector Gravity = FVector(0.0f,0.0f,-980.0f); 
	TArray<float> SuspensionPrevDistance = { 0.0f,0.0f,0.0f,0.0f };
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
	FVector InertialTensor = FVector(10000.0f, 10000.0f, 10000.0f); // tweak manually;
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



};
