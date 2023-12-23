// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "SimpleRoad.generated.h"


UCLASS()
class DRIVING_API ASimpleRoad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASimpleRoad();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float roadWidth;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;


public:
	UPROPERTY(VisibleAnywhere, Category = "Spline")
	USplineComponent* RoadSpline;

};
