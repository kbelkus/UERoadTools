// Fill out your copyright notice in the Description page of Project Settings.
#include "SimpleRoad.h"

// Sets default values
ASimpleRoad::ASimpleRoad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	RoadSpline = CreateDefaultSubobject<USplineComponent>("RoadSpline");
	//if (RoadSpline)
	//{
	//	SetRootComponent(RoadSpline);
	//}

}

// Called when the game starts or when spawned
void ASimpleRoad::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASimpleRoad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

