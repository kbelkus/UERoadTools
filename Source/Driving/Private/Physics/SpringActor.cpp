// Fill out your copyright notice in the Description page of Project Settings.


#include "Physics/SpringActor.h"

// Sets default values
ASpringActor::ASpringActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent->SetMobility(EComponentMobility::Movable);
	SetRootComponent(RootComponent);

}

// Called when the game starts or when spawned
void ASpringActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpringActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FlushPersistentDebugLines(GetWorld());

	//Draw Anchor
	DrawDebugSphere(GetWorld(), this->GetActorLocation(), 20.0f, 16, FColor::Red, true, 1.0f, 1, 1.0f);

	float CurrentLength = Offset.Size();


	//Hookes Law - F = -k * x = -k Spring Constant x = displacement
	float X = CurrentLength - RestLength; //Get Our Displacement 

	//SpringConstant is 1
	float Force = -SpringConstant * X; //Calculate Spring Force 

	//Newtons Second Law -- Assume Mass = 1 F = M * A
	float Acc = (Force / Mass) + Gravity.Z; //Integrate Mass and Graviy

	Velocity +=Acc * DeltaTime;
	Velocity *= 0.985f;

	Offset += FVector(0, 0,Velocity * DeltaTime);

	//Draw Bob
	DrawDebugSphere(GetWorld(), this->GetActorLocation() + Offset, 20.0f, 16, FColor::Green, true, 1.0f, 1, 1.0f);

}

