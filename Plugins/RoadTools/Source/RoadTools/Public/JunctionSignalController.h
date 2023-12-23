// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LaneSpline.h"
#include "GameFramework/Actor.h"
#include "JunctionSignalController.generated.h"

USTRUCT()
struct FSignalPhase
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		float PhaseLength;
	UPROPERTY(EditAnywhere)
		TArray<ALaneSpline*> LaneGroupProceed;
	UPROPERTY(EditAnywhere)
		TArray<ALaneSpline*> LaneGroupStop;

	FSignalPhase()
	{

	}

};

UCLASS()
class ROADTOOLS_API AJunctionSignalController : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AJunctionSignalController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere)
	TArray<FSignalPhase> Phases;

	//Editor Controls
	bool UpdateSplines = false; //Grab all splines within the volume

	FVector Center;
	FVector Extent;


	UFUNCTION()
	void UpdateAllConnectedJunctions();

	float GlobalTime;
	float PhaseTimer = 0.0f; //How many seconds into the phase are we
	int PhaseIndex = 0;		 //Which index of the phase are we in


	UFUNCTION()
	void DrawDebugBounds();

	virtual void OnConstruction(const FTransform& Transform) override;




	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
