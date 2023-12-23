// Fill out your copyright notice in the Description page of Project Settings.

#include "JunctionSignalController.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"

// Sets default values
AJunctionSignalController::AJunctionSignalController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);


}

void AJunctionSignalController::OnConstruction(const FTransform& Transform)
{

	Center = this->GetActorLocation();
	Extent = this->GetActorScale();

	//Draw Debug Object
	//FlushPersistentDebugLines(GetWorld());
	DrawDebugBox(GetWorld(), Center, Extent, FColor::Blue, true, -1.0, 1, 10.0f);



}


void AJunctionSignalController::DrawDebugBounds()
{




}



//When called loop through all connected splines and set their lane status
void AJunctionSignalController::UpdateAllConnectedJunctions()
{



}


// Called when the game starts or when spawned
void AJunctionSignalController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AJunctionSignalController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	//PhaseTimer = PhaseTimer + DeltaTime;

	//float CurrentPhaseLength = Phases[PhaseIndex].PhaseLength;

	//if(PhaseTimer >= CurrentPhaseLength)



}

