// Fill out your copyright notice in the Description page of Project Settings.


#include "SandboxActorForTEsting.h"

// Sets default values
ASandboxActorForTEsting::ASandboxActorForTEsting()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}



void ASandboxActorForTEsting::OnConstruction(const FTransform& Transform)
{

	Super::OnConstruction(Transform);


	FlushPersistentDebugLines(GetWorld());
	DrawDebugObjects();

	if(DrawDebug)
	{
	
	}

}


void ASandboxActorForTEsting::DrawDebugObjects()
{


	DrawDebugSphere(GetWorld(), Location, 100.0f, 10.0f, FColor::Green, true, -1.0f, 2.0, 10.0f);

	DrawDebugSphere(GetWorld(), Location + FVector(500, 0, 0), 100.0f, 10.0f, FColor::Blue, true, -1.0f, 2.0, 10.0f);

	DrawDebugBox(GetWorld(), Center, Extents, FColor::Red, true, -1.0f, 2, 1.0f);
	DrawDebugBox(GetWorld(), CenterTwo, Extents, FColor::Red, true, -1.0f, 2, 1.0f);
}



// Called when the game starts or when spawned
void ASandboxActorForTEsting::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASandboxActorForTEsting::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

