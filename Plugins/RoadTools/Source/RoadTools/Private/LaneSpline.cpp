// Fill out your copyright notice in the Description page of Project Settings.


#include "LaneSpline.h"

// Sets default values
ALaneSpline::ALaneSpline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	LaneSpline = CreateDefaultSubobject<USplineComponent>("LaneSpline");
	LaneSpline->SetupAttachment(GetRootComponent());

}

void ALaneSpline::OnConstruction(const FTransform& RootTransform)
{
	FlushPersistentDebugLines(GetWorld());

	if (DrawDebug)
	{
		DebugDrawLaneDirection();


		if (LaneConnections.Num() != 0)
		{

			UE_LOG(LogTemp, Warning, TEXT("ENTERED HERE SOMEHOW"))
			DebugDrawLaneConnections();
		}
	}


	if (FlipLaneDirection)
	{
		ReverseSpline();
		FlipLaneDirection = false;
	}


	LaneStartPosition = LaneSpline->GetLocationAtSplineInputKey(0,ESplineCoordinateSpace::World);


}

//Utility for flipping spline direction
void ALaneSpline::ReverseSpline()
{
	//TArray<FVector>SplinePoints;
	TArray<FSplinePoint> SplinePoints;
	int SplinePointCount = LaneSpline->GetNumberOfSplinePoints();

	FVector ActorLocation = this->GetActorLocation();

	//Go through the current spline and add each point to an array
	for (int i = 0; i < SplinePointCount; i++)
	{

		FSplinePoint Point;

		Point.Position = LaneSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World) - ActorLocation;
		Point.Rotation = LaneSpline->GetRotationAtSplinePoint(i, ESplineCoordinateSpace::World);
		Point.Scale = LaneSpline->GetScaleAtSplinePoint(i);
		Point.ArriveTangent = LaneSpline->GetArriveTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
		Point.LeaveTangent = LaneSpline->GetLeaveTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
		//Point.Type = LaneSpline->GetSplinePointType(i);
		//Point.InputKey = SplinePointCount - i;


		//Point.Position = FVector(0.0f,100 * i,0.0f);
		//Point.Rotation = LaneSpline->GetRotationAtSplinePoint(i, ESplineCoordinateSpace::World);
		Point.InputKey = (SplinePointCount - 1) - i;

		SplinePoints.Add(Point);

		//UE_LOG(LogTemp, Warning, TEXT("Int Positive I, %i"), i);

	}
	
	LaneSpline->ClearSplinePoints(true);

	//Read all our points in a reverse order

	UE_LOG(LogTemp, Warning, TEXT("Int Positive I, %i"), SplinePoints.Num());


	//LaneSpline->AddPoints(SplinePoints, true);

	for (int i = 0; i < SplinePoints.Num(); i++)
	{

		LaneSpline->AddPoint(SplinePoints[i], true);


	}



	//for (int i = (SplinePointCount - 1); i >=0; --i)
	//{

	//	FSplinePoint Point;
	//	Point = SplinePoints[i];
	//	Point.InputKey = SplinePointCount - i;


	//	LaneSpline->AddPoint(SplinePoints[i], true);


	//	UE_LOG(LogTemp, Warning, TEXT("Input Keu, %i"), SplinePointCount - i);
	//}


}



void ALaneSpline::DebugDrawLaneDirection()
{
	if (LaneSpline->GetNumberOfSplinePoints() > 1)
	{
		float SplineLength = LaneSpline->GetSplineLength();
		float increment = 1.0f / 10.0f;

		for (int i = 0; i < 10; i++)
		{
			FVector StartPosition = LaneSpline->GetWorldLocationAtTime(increment * i);
			FVector EndPosition = LaneSpline->GetWorldLocationAtTime(increment * (i + 1));

			DrawDebugDirectionalArrow(GetWorld(), StartPosition, EndPosition,10000.0f, FColor::Blue, true, -1.0f, 2, 10.0f);
			DrawDebugPoint(GetWorld(), StartPosition, 20.0f, FColor::Red, true, -1.0f, 10);

		}
	}
}


void ALaneSpline::DebugDrawLaneConnections()
{

	if (LaneConnections.Num() == 0 || LaneConnections[0].LaneReference == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can not draw connected lanes"))
		return;
	}


	int LaneConnectionCount = LaneConnections.Num();


	//For each connected spline - draw a series of points to show which one is connected
	for (int i = 0; i < (LaneConnectionCount); i++)
	{

		if (LaneConnections[i].LaneReference == nullptr)
		{
			return;
		}

		ALaneSpline* CurrentSpline = LaneConnections[i].LaneReference;
		USplineComponent* SampledLaneSpline = CurrentSpline->ReturnSpline();

		if (SampledLaneSpline)
		{
			UE_LOG(LogTemp, Warning, TEXT("Retrieved Spline"));

			FString SplineName = SampledLaneSpline->GetName();
			UE_LOG(LogTemp, Warning, TEXT("Spline Name, %s"), *SplineName);


			float SplineLength = SampledLaneSpline->GetSplineLength();
			float increment = 1.0f / 10.0f;


			for (int j = 0; j < 10; j++)
			{
				FVector StartPosition = SampledLaneSpline->GetWorldLocationAtTime(increment * j);
				DrawDebugPoint(GetWorld(), StartPosition, 20.0f, DebugColours[i], true, -1.0f, 10);
				//UE_LOG(LogTemp, Warning, TEXT("IN LOOP"));

			}

		}

	}

}



void ALaneSpline::DebugDrawLaneStatus()
{

	//FlushPersistentDebugLines(GetWorld());
	DrawDebugSphere(GetWorld(), LaneStartPosition, 500.0f, 16, LaneStatusColors[LaneStatus], true, -1.0f, 1, 10.0f);


}


void ALaneSpline::UpdateSplineStatus()
{

	if (LaneSpline == nullptr)
	{
		return;
	}


	LaneSpline->EditorUnselectedSplineSegmentColor = LaneStatusColors[LaneStatus];

}





// Called when the game starts or when spawned
void ALaneSpline::BeginPlay()
{

	LaneStartPosition = LaneSpline->GetLocationAtSplineInputKey(0, ESplineCoordinateSpace::World);

	Super::BeginPlay();
	
}

// Called every frame
void ALaneSpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	//Temp for now,move to traffic lights / Update in its own step
	UpdateSplineStatus();
	DebugDrawLaneStatus();


}

