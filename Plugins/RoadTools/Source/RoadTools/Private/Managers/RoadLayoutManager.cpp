// Copyright Kieran Belkus 2025

#include "Managers/RoadLayoutManager.h"
#include "Kismet/GameplayStatics.h"
#include "LaneSpline.h"

ARoadLayoutManager::ARoadLayoutManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARoadLayoutManager::BuildRoadNetwork()
{
	TObjectPtr<UWorld> WorldContext = this->GetWorld();

	TArray<ALaneSpline*> WorldLaneSplines = GetAllSceneSplines(WorldContext);

	LaneSplines.Empty();
	LaneSplines = BuildRoadNetworkSceneData(WorldLaneSplines);
	
	ConnectAllLaneSplines(WorldLaneSplines);
}

TArray<FRoadNetworkLanes> ARoadLayoutManager::BuildRoadNetworkSceneData(TArray<ALaneSpline*> InLaneSplines)
{
	TArray<FRoadNetworkLanes> BuildNetworkLanes = TArray<FRoadNetworkLanes>();

	for (TObjectPtr<ALaneSpline> CurrentLaneSpline : InLaneSplines)
	{
		FRoadNetworkLanes NewLaneSpline;

		NewLaneSpline.LaneSpline = CurrentLaneSpline;
		NewLaneSpline.SplineLength = CurrentLaneSpline->LaneSpline->GetSplineLength();
		NewLaneSpline.BoundingBox = CurrentLaneSpline->GetComponentsBoundingBox();

		BuildNetworkLanes.Add(NewLaneSpline);
	}

	return BuildNetworkLanes;
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

FRoadNetworkLanes ARoadLayoutManager::GetRoadSplineData(int InID)
{
	FRoadNetworkLanes LaneSpline = FRoadNetworkLanes();

	if (!LaneSplines.IsEmpty() && LaneSplines.IsValidIndex(InID))
	{
		LaneSpline = LaneSplines[InID];
	}

	return LaneSpline;
}

ALaneSpline* ARoadLayoutManager::LoadLaneSpline(FRoadNetworkLanes InRoadNetworkLane)
{
	TObjectPtr<ALaneSpline> LoadedLaneSpline = InRoadNetworkLane.LaneSpline.LoadSynchronous();

	return LoadedLaneSpline;
}

FRoadNetworkLanes ARoadLayoutManager::GetNearestRoadSpline(const FVector InLocation, bool& FoundSpline, float& NearestDistanceAlongSpline)
{
	//Loop through road splines. Get Bounding Boxes. Then, if multiple get nearest point and return spline and position along it.
	TArray<FRoadNetworkLanes> ValidLaneSplines = TArray<FRoadNetworkLanes>();
	float ResolvedNearestDistanceAlongSpline = 0.0f;
	
	for (FRoadNetworkLanes CurrentLaneSpline : LaneSplines)
	{
		if (CurrentLaneSpline.BoundingBox.IsInsideOrOnXY(InLocation))
		{
			ValidLaneSplines.Add(CurrentLaneSpline);
		}
	}

	if (ValidLaneSplines.IsEmpty())
	{
		FoundSpline = false;
		NearestDistanceAlongSpline = 0.0f;

		return FRoadNetworkLanes();
	}

	if (ValidLaneSplines.Num() == 1)
	{
		FoundSpline = true;

		TObjectPtr<ALaneSpline> LaneSpline =ValidLaneSplines[0].LaneSpline.LoadSynchronous();

		if (LaneSpline)
		{
			ResolvedNearestDistanceAlongSpline = LaneSpline->LaneSpline->GetDistanceAlongSplineAtLocation(InLocation, ESplineCoordinateSpace::World);
		}

		NearestDistanceAlongSpline = ResolvedNearestDistanceAlongSpline;

		return ValidLaneSplines[0];
	}
	else
	{
		//Check our spline points and try to find which keypoint is closest to our in location
		float MaximumDistance = 100000.0f;
		FRoadNetworkLanes NearestRoadSpline = FRoadNetworkLanes();
		int NearestSplinePoint = 0.0f;
		float UValueAlongSpline = 0.0f;

		for (FRoadNetworkLanes CurrentLaneSpline : ValidLaneSplines)
		{
			TObjectPtr<ALaneSpline> LaneSpline = CurrentLaneSpline.LaneSpline.LoadSynchronous();

			if (LaneSpline)
			{
				for (int i = 0; i < LaneSpline->LaneSpline->GetNumberOfSplinePoints(); i++)
				{
					float CurrentDistance = FVector::Distance(LaneSpline->LaneSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World), InLocation);

					if (CurrentDistance < MaximumDistance)
					{
						NearestRoadSpline = CurrentLaneSpline;
						NearestSplinePoint = i;
						UValueAlongSpline = LaneSpline->LaneSpline->GetDistanceAlongSplineAtSplinePoint(i);
						UE_LOG(LogTemp, Log, TEXT("RoadLayoutManager: GetNearestRoad Distance Along %f"), UValueAlongSpline);

						MaximumDistance = CurrentDistance;
					}
				}
			}
		}

		NearestDistanceAlongSpline = UValueAlongSpline;
		FoundSpline = true;

		return NearestRoadSpline;
	}


	return FRoadNetworkLanes();
}

void ARoadLayoutManager::TestGetNearestLaneSplineLocation()
{
	UE_LOG(LogTemp, Log, TEXT("RoadLayoutManager: TestGetNearestLaneSplineLocation"));

	bool FoundLaneSpline = false;
	float DistanceAlongSpline = 0.0f;

	FRoadNetworkLanes FoundRoadSpline = GetNearestRoadSpline(InTestLocation, FoundLaneSpline, DistanceAlongSpline);

	if (FoundLaneSpline)
	{
		UE_LOG(LogTemp, Log, TEXT("RoadLayoutManager: Test: Found Road Spline"));

		UE_LOG(LogTemp, Log, TEXT("RoadLayoutManager: Test: Name: %s,  Distance Along Spline: %f"),*FoundRoadSpline.LaneSpline->GetFullName(), DistanceAlongSpline);
		
		FVector NearestLocation = FoundRoadSpline.LaneSpline->LaneSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);

		DrawDebugSphere(GetWorld(), NearestLocation, 20.0f, 16, FColor::Green, true, 10.0f, 2, 1.0f);
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

