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


USTRUCT()
struct FSignalDebugRenderer
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Position;
	UPROPERTY()
	FVector Direction;
	UPROPERTY()
	FColor Colour;

	FSignalDebugRenderer()
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

	//Editor Debug Controls -- THis is just to store some properties so the debug draw stuff is visible in the editor
	//These arnt needed for the game (DrawComponent Vizualizer is also now missing sphere, cone etc
	TArray<FSignalDebugRenderer> SignalDebugObjects;
	TArray<FVector> LaneProceedPointPositions;

	FTimerHandle PhaseTimeHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UpdateSignals;


public:

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//class USceneComponent* SceneComponent;
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

	//EDITOR DEBUG FUNCTIONS
	UFUNCTION()
	void DrawDebugBounds();
	UFUNCTION()


	void DrawPhaseDebug();
	UFUNCTION()
	void DrawTrafficDebug();
	UFUNCTION()
	void DrawLaneDebugPoints();


	UFUNCTION()
	void UpdateLanes(int Phase);

	UFUNCTION()
	void UpdatePhaseTimer();

	//EDITOR DEBUG CONTROLS
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DrawDebug;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int DebugPhaseIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Length = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleWidth = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleHeight = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isEnabled;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector DebugSignalHeight = FVector(0,0,400.0f);


	virtual void OnConstruction(const FTransform& Transform) override;
	//void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;


	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
