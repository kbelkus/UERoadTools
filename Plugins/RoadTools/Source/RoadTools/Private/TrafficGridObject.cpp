// Fill out your copyright notice in the Description page of Project Settings.


#include "TrafficGridObject.h"
#include "LaneSpline.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"

// Sets default values
ATrafficGridObject::ATrafficGridObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionBounds = CreateDefaultSubobject<UBoxComponent>("CollisionBounds");
	CollisionBounds->SetupAttachment(GetRootComponent());
	CollisionBounds->SetCollisionResponseToChannel(ECollisionChannel::ECC_OverlapAll_Deprecated, ECollisionResponse::ECR_Overlap);

	//CollisionBounds->OnComponentBeginOverlap.AddDynamic(this, &ATrafficGridObject::OnOverlapBegin);
	//CollisionBounds->OnComponentEndOverlap.AddDynamic(this, &ATrafficGridObject::OnOverlapEnd);

}

void ATrafficGridObject::OnConstruction(const FTransform& RootTransform)
{
	Super::OnConstruction(RootTransform);

	Location = this->GetActorLocation();
	DrawDebugBox(GetWorld(), Location, CellSize, FColor::Blue, true, 1000.0f, 1,20.0f);

	if (UpdateLanes == true)
	{
		GetAllIntersectingLanes();
		UpdateLanes = false;
	}


	if (DebugDrawLanes == true)
	{
		FlushPersistentDebugLines(GetWorld());

		DebugDrawConnectedLanes();

		DebugDrawLanes = false;
	}


	if (DebugDrawCells == true)
	{
		FlushPersistentDebugLines(GetWorld());

		DebugDrawNeighbourCells();

		DebugDrawCells = false;
	}


}

void ATrafficGridObject::SetBoxExtent(FVector Extent)
{
	if (CollisionBounds)
	{
		CollisionBounds->SetBoxExtent(Extent);
	}

}


void ATrafficGridObject::GetAllIntersectingLanes()
{
	AssociatedLanes.Empty();

	TArray<AActor*> OverlappingActors;

	CollisionBounds->GetOverlappingActors(OverlappingActors, ALaneSpline::StaticClass());

	UE_LOG(LogTemp, Warning, TEXT("Inside Found Actors, %i"), OverlappingActors.Num());

	for (AActor* Actor : OverlappingActors)
	{

		ALaneSpline* CurrentLane = Cast<ALaneSpline>(Actor);
		if (CurrentLane)
		{
			AssociatedLanes.Add(CurrentLane);
		}
	}
}

//For Editor use to debug draw lanes
void ATrafficGridObject::DebugDrawConnectedLanes()
{

	int RoadCount = ConnectedRoads.Num();

	DrawPointsPositions.Empty();

	for (int i = 0; i < RoadCount; i++)
	{
		//For Each Road, and for each point add its location
		USplineComponent* SplineComponent = ConnectedRoads[i].ReferencedLaneSpline->ReturnSpline();

		//For Each Point in our overlapping array, get its location
		for (int j = 0; j < ConnectedRoads[i].PointID.Num(); j++)
		{
			FVector SelectedPointLocation = SplineComponent->GetWorldLocationAtSplinePoint(ConnectedRoads[i].PointID[j]);
			DrawPointsPositions.Add(SelectedPointLocation);
		}
	}

	//For Each Point in our new array, draw a point there so we can see which lanes are referenced by this cell
	for (int i = 0; i < DrawPointsPositions.Num(); i++)
	{
		DrawDebugPoint(GetWorld(), DrawPointsPositions[i], 30.0f, FColor::Green, true, -1.0f, 5);
	}

}



void ATrafficGridObject::DebugDrawNeighbourCells()
{

	FVector StartLocation = this->GetActorLocation();


	for (int i = 0; i < NeighbourCells.Num(); i++)
	{


		FVector EndLocation = NeighbourCells[i]->GetActorLocation();

		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, true, -1.0f, 2, 30.0f);
		DrawDebugPoint(GetWorld(), EndLocation, 30.0f, FColor::Blue, true, -1.0f, 2);


	}
}


//void ATrafficGridObject::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
//{
//
//	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
//
//	//UE_LOG(LogTemp, Warning, TEXT("Inside Cell, %i"), 0);
//
//	if (PlayerPawn == OtherActor)
//	{
//		isActive = true;
//		//int CellID = 0;
//
//		UE_LOG(LogTemp, Warning, TEXT("Inside Cell, %i"), CellID);
//		
//	}
//
//}



//void ATrafficGridObject::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//
//
//	UE_LOG(LogTemp, Warning, TEXT("Left Cell, %i"), CellID);
//	isActive = false;
//}

void ATrafficGridObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATrafficGridObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

