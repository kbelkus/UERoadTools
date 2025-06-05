// Fill out your copyright notice in the Description page of Project Settings.

#include "TrafficManager/TrafficManagerGrid.h"

//Notes:
//Player needs to get this manager and set which cell the player is in.
//Also set the 'active' cells around it. This will act as the traffic max distance bounds
//Traffic when changing lanes should query which cell they're in if the new cell is inactive - spawn a timer then remove the car. 
//This should in theory keep our traffic roughly near the player
//Traffic Manager should handle the pooling, removal and spawning of vehicles (Do this via active cells, check player view? Check lane direction TO player?)

ATrafficManagerGrid::ATrafficManagerGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent->SetMobility(EComponentMobility::Movable);
	SetRootComponent(RootComponent);

}

void ATrafficManagerGrid::GenerateTrafficCells()
{
	//Build a bunch of cells that we'll use to cull traffic
	//Vehicles should be aware of the cell they're in, then





}

// Called when the game starts or when spawned
void ATrafficManagerGrid::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATrafficManagerGrid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	PreviewBoundsTransform.SetLocation(BoundingVolumeLocation);
	PreviewBoundsTransform.SetScale3D(BoundingVolumeExtents);

	DrawDebugBox(GetWorld(), PreviewBoundsTransform.GetLocation(), PreviewBoundsTransform.GetScale3D(), FColor::Red, true, -1.0f, 1, 20.0f);

}

// Called every frame
void ATrafficManagerGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

