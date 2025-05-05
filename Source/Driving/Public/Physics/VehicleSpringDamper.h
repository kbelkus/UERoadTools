// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VehicleSpringDamper.generated.h"

class ALaneSpline;
class USplineComponent;

UCLASS()
class DRIVING_API AVehicleSpringDamper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVehicleSpringDamper();

	//Components
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	TArray<TObjectPtr<UStaticMeshComponent>> WheelMeshes;


	UPROPERTY(EditAnywhere)
	TArray<FVector> WheelOffset = { FVector(150,100,0), FVector(150,-100,0), FVector(-150,100,0), FVector(-150,-100,0) };

	UPROPERTY(EditAnywhere)
	TArray<float> WheelIDs = { 1.0f, 1.0f, 0.0f, 0.0f};

	UPROPERTY(EditAnywhere)
	TArray<float> OldDistance = { 0.0f,0.0f,0.0f,0.0f };

	//Suspension
	UPROPERTY(EditAnywhere)
	float MaxSuspesionLength = 100.0f;

	UPROPERTY(EditAnywhere)
	float VehicleMass = 2000.0f;

	UPROPERTY(EditAnywhere)
	float WheelRadius = 50.0f;

	UPROPERTY(EditAnywhere)
	float SuspensionOffset = 0.0f;

	//UPROPERTY(EditAnywhere)
	//float SuspensionMultiplier = 12000.0f;

	//UPROPERTY(EditAnywhere)
	//float SpringDampening = 50000.0f;

	//UPROPERTY(EditAnywhere)
	//float MaxDampening = 400.0f;

	UPROPERTY(EditAnywhere)
	FTransform CarBodyTransform;

	
	//NewTest
	UPROPERTY(EditAnywhere)
	float RestLength = 100.0f;

	UPROPERTY(EditAnywhere)
	float SpringStiffness = 2000.0f;

	UPROPERTY(EditAnywhere)
	float DampingCoefficient = 2000.0f;

	UPROPERTY(EditAnywhere)
	float IntertialTensorScale = 100.0f;

	FVector VehicleVelocity;
	FVector VehicleAngularVelocity;

	UPROPERTY(EditAnywhere)
	FVector InertialTensor = FVector(1.0f, 1.0f, 1.0f); // tweak manually

	FQuat ActorRotation;

	UPROPERTY(EditAnywhere)
	FVector CalculatedCOM;

	//DEBUG CONTROLS
	UPROPERTY(EditAnywhere)
	FVector BoxExtents = FVector(300, 200, 180);

	UPROPERTY(EditAnywhere)
	FVector BoxLocation = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere)
	bool ApplyLocomotion = false;

	UPROPERTY(EditAnywhere)
	float MaxSpeed = 10.0f;

	UPROPERTY(EditAnywhere)
	float VehicleAcc = 1.0f;

	//TEMP Get Spline
	UPROPERTY(EditAnywhere)
	bool bUseSpline;

	UPROPERTY(EditAnywhere)
	TObjectPtr<ALaneSpline> OptionalLaneSpline;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USplineComponent> ActiveSplineComponent;

	UPROPERTY(EditAnywhere)
	float SplineDelta;

	UPROPERTY(EditAnywhere)
	FTransform SplineTransform;

	//Speed Delta
	UPROPERTY(EditAnywhere)
	FVector PreviousLocation;
	
	UPROPERTY(EditAnywhere)
	float CurrentSpeed;

	float WheelCircumference;

	UPROPERTY(EditAnywhere)
	float SteerAngle;

	UPROPERTY(EditAnywhere)
	float TempRotationDelta;

	UPROPERTY(EditAnywhere) //Can Remove this
	float RotationDelta;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	FTransform GetPositionAlongSpline(float InSplineDelta, FTransform& OutTargetTransform);

	UFUNCTION()
	void UpdateWheelRotations(float InSpeedDelta, FTransform InSteeringTarget, float DeltaTime, FVector InVehicleForwarDirection, FVector InVehicleLocation);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Input Meshes
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UStaticMesh> InputBodyMesh;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	TObjectPtr<UStaticMesh> WheelMesh;
	
};
