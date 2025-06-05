// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrafficManagerGrid.generated.h"

UCLASS()
class DRIVING_API ATrafficManagerGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrafficManagerGrid();

	UFUNCTION(CallInEditor)
	void GenerateTrafficCells();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	FVector BoundingVolumeExtents;

	UPROPERTY(EditAnywhere)
	FVector BoundingVolumeLocation;

	UPROPERTY(EditAnywhere)
	int GridResolution;

	UPROPERTY(EditAnywhere)
	TArray<FBox3d> BoundingBoxArrays;

	UPROPERTY(EditAnywhere)
	FTransform PreviewBoundsTransform;

};
