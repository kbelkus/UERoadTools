// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JunctionSurface.h"
#include "JunctionSurfaceComponent.generated.h"




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROADTOOLS_API UJunctionSurfaceComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UJunctionSurfaceComponent();

	TArray<FJunctionPoint> GetJunctionPoints() const { return JunctionPoints; }
	FVector GetJunctionCenterLocation() const { return JunctionCenterLocation; }

protected:


public:	

	UPROPERTY(VisibleAnywhere, Category = "Junction Tool Component Data")
	TArray<FJunctionPoint> JunctionPoints;

	UPROPERTY(VisibleAnywhere, Category = "Junction Tool Component Data")
	FVector JunctionCenterLocation;
		
};
