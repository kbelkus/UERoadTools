// Copyright Kieran Belkus 2025
// Utility Manager for automatically connecting RoadSplines together. For now, its entirely brute force, but will move to a grid later

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadLayoutManager.generated.h"

class ALaneSpline;

UCLASS()
class ROADTOOLS_API ARoadLayoutManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoadLayoutManager();

	UPROPERTY(EditAnywhere)
	float SplineSearchDistance = 100.0f;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(CallInEditor)
	void BuildRoadNetwork();

	UFUNCTION()
	TArray<ALaneSpline*> GetAllSceneSplines(UWorld* InWorldContext);

	UFUNCTION()
	void ConnectAllLaneSplines(TArray<ALaneSpline*> InLaneSplines);

};
