// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SandboxActorForTEsting.generated.h"


UCLASS()
class ROADTOOLS_API ASandboxActorForTEsting : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASandboxActorForTEsting();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;





public:	

	UPROPERTY(EditAnywhere)
	bool DrawDebug;

	UFUNCTION()
	void DrawDebugObjects();

	UPROPERTY(EditAnywhere)
	FVector Location = FVector(0, 0, 0);
	UPROPERTY(EditAnywhere)
	FVector Center = FVector(0, 0, 0);
	UPROPERTY(EditAnywhere)
	FVector CenterTwo = FVector(0, 0, 0);
	UPROPERTY(EditAnywhere)
	FVector Extents = FVector(100.0f, 100.0f, 100.0f);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
