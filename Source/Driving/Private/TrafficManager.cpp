// Fill out your copyright notice in the Description page of Project Settings.


#include "TrafficManager.h"

// Sets default values
ATrafficManager::ATrafficManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}



void ATrafficManager::OnConstruction(const FTransform& RootTransform)
{
}



// Called when the game starts or when spawned
void ATrafficManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATrafficManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

