// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIVehicleDynamicsUtilities.generated.h"

/**
 * 
 */
USTRUCT()
struct FVehicleDynamicsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "WheelData")
	TArray<FVector> WheelOffsets =
	{
		FVector(150.0f,75.0f,0.0f),
		FVector(150.0f,-75.0f,0.0f),
		FVector(-100.0f,75.0f,0.0f),
		FVector(-100.0f,-75.0f,0.0f)
	};

	UPROPERTY(EditAnywhere, Category = "WheelData")
	TArray<FVector> WheelScales =
	{
		FVector(1.0f,1.0f,-1.0f),
		FVector(1.0f,1.0f,1.0f),
		FVector(1.0f,1.0f,-1.0f),
		FVector(1.0f,1.0f,1.0f)
	};

	UPROPERTY(EditAnywhere, Category = "WheelData")
	TArray<int> WheelMask =
	{
		1,1,0,0
	};

	UPROPERTY(EditAnywhere, Category = "Visual Meshes")
	TSoftObjectPtr<UStaticMesh> WheelMeshes;

	UPROPERTY(EditAnywhere, Category = "Visual Meshes")
	TSoftObjectPtr<UStaticMesh> BodyMesh;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float VehicleMaxSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float VehicleMass = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float CarBodyOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float SuspensionOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float MaxSuspensionLength = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float SuspensionRestLength = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float SuspensionSpringStiffness = 20000.0f;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float SuspensionDampingCoefficient = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float WheelRadius = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	FVector InertialTensor = FVector(10000.0f, 10000.0f, 10000.0f);

	UPROPERTY(EditAnywhere, Category = "Dynamics")
	float IntertialTensorScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FVector VehicleExtends = FVector(200, 100, 50);

	UPROPERTY(EditAnywhere, Category = "Debug")
	FVector VehicleExtendsOffset = FVector(200, 100, 50);

};



UCLASS()
class DRIVING_API UVehicleDynamicsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVehicleDynamicsData VehicleDynamics;

};

class DRIVING_API AIVehicleDynamicsUtilities
{
public:
	AIVehicleDynamicsUtilities();
	~AIVehicleDynamicsUtilities();
};
