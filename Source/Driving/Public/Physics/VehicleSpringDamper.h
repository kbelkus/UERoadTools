// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VehicleSpringDamper.generated.h"

UCLASS()
class DRIVING_API AVehicleSpringDamper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVehicleSpringDamper();

	UPROPERTY(EditAnywhere)
	TArray<FVector> WheelOffset = { FVector(150,100,0), FVector(150,-100,0), FVector(-150,100,0), FVector(-150,-100,0) };

	UPROPERTY(EditAnywhere)
	TArray<float> OldDistance = { 0.0f,0.0f,0.0f,0.0f };

	//Suspension
	UPROPERTY(EditAnywhere)
	float MaxSuspesionLength = 150.0f;

	UPROPERTY(EditAnywhere)
	float VehicleMass = 1000.0f;

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
	float RestLength = 200.0f;

	UPROPERTY(EditAnywhere)
	float SpringStiffness = 5000.0f;

	UPROPERTY(EditAnywhere)
	float DampingCoefficient = 800.0f;

	FVector VehicleVelocity;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
