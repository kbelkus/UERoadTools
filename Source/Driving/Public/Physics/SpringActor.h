// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpringActor.generated.h"

UCLASS()
class DRIVING_API ASpringActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpringActor();

	UPROPERTY(EditAnywhere)
	FVector Offset = FVector(0, 0, -200); //Displacment (X)

	UPROPERTY(EditAnywhere)
	float SpringConstant = 1.0f; //(K)

	UPROPERTY(EditAnywhere)
	float RestLength = 300; 

	UPROPERTY(EditAnywhere)
	float Velocity = 0.0f;

	UPROPERTY(EditAnywhere)
	FVector Gravity = FVector(0.0f, 0.0f, -980.0f);

	UPROPERTY(EditAnywhere)
	float Mass = 1.0f;
		 


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
