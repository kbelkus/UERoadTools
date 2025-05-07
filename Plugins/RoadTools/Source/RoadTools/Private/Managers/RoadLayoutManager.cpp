// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/RoadLayoutManager.h"
#include "Kismet/GameplayStatics.h"
#include "LaneSpline.h"

// Sets default values
ARoadLayoutManager::ARoadLayoutManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ARoadLayoutManager::BuildRoadNetwork()
{

	TObjectPtr<UWorld> WorldContext = this->GetWorld();

	TArray<ALaneSpline*> WorldLaneSplines = GetAllSceneSplines(WorldContext);

	ConnectAllLaneSplines(WorldLaneSplines);
}

TArray<ALaneSpline*> ARoadLayoutManager::GetAllSceneSplines(UWorld* InWorldContext)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(InWorldContext, ALaneSpline::StaticClass(), FoundActors);

	TArray<TObjectPtr<ALaneSpline>> WorldLaneSplines;

	for (TObjectPtr<AActor> LaneSpline : FoundActors)
	{
		TObjectPtr<ALaneSpline> CurrentLaneSpline = Cast<ALaneSpline>(LaneSpline);

		if (CurrentLaneSpline)
		{
			WorldLaneSplines.Add(CurrentLaneSpline);
		}
	}

	return WorldLaneSplines;
}

void ARoadLayoutManager::ConnectAllLaneSplines(TArray<ALaneSpline*> InLaneSplines)
{
	UE_LOG(LogTemp, Log, TEXT("RoadLayoutManager::ConnectAllLaneSpline"));

	for (TObjectPtr<ALaneSpline> CurrentLaneSpline : InLaneSplines)
	{
		CurrentLaneSpline->LaneConnections.Empty();

		FVector StartLocation = CurrentLaneSpline->LaneSpline->GetLocationAtSplinePoint(CurrentLaneSpline->LaneSpline->GetNumberOfSplinePoints(),ESplineCoordinateSpace::World);

		for (TObjectPtr<ALaneSpline> TargetLaneSpline : InLaneSplines)
		{
			if (CurrentLaneSpline == TargetLaneSpline)
			{
				continue;
			}

			FVector SpineEndTargetLocation = TargetLaneSpline->LaneSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

			//Do we need to handle reverse connections here?
			if (FVector::Distance(StartLocation, SpineEndTargetLocation) < SplineSearchDistance)
			{
				FLaneConnections NewLaneConnection;

				NewLaneConnection.LaneReference = TargetLaneSpline;
				NewLaneConnection.ConnectionPosition = 0;

				CurrentLaneSpline->LaneConnections.Add(NewLaneConnection);
			}
		}
	}
	
}

// Called when the game starts or when spawned
void ARoadLayoutManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARoadLayoutManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

