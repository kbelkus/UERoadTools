// Fill out your copyright notice in the Description page of Project Settings.
#include "CurveSandbox.h"
#include "GenericPlatform/GenericPlatformMath.h"


// Sets default values
ACurveSandbox::ACurveSandbox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	Spline1 = CreateDefaultSubobject<USplineComponent>("First Spline");
	Spline1->SetupAttachment(GetRootComponent());


	Spline2 = CreateDefaultSubobject<USplineComponent>("Second Spline");
	Spline2->SetupAttachment(GetRootComponent());



}


void ACurveSandbox::OnConstruction(const FTransform& RootTransform)
{
	Super::OnConstruction(RootTransform);


	FlushPersistentDebugLines(GetWorld());

	int SplinePointCount = Spline1->GetNumberOfSplinePoints();
	int SplineSegmentCount = Spline1->GetNumberOfSplineSegments();
	
	UE_LOG(LogTemp, Warning, TEXT("Spline Count: %d"), SplinePointCount);
	UE_LOG(LogTemp, Warning, TEXT("Segment Count: %d"), SplineSegmentCount);


	//Make Bounding Box of each curve.
	//GetLeaveTangentAtSplinePoint
	FVector TangentPoint = Spline1->GetLeaveTangentAtSplinePoint(1, ESplineCoordinateSpace::World);

	UE_LOG(LogTemp, Warning, TEXT("Spline Tangent P: %s"), *TangentPoint.ToString());

	//Make a polygon around segment

	for (int i = 0; i < SplineSegmentCount; i++)
	{
		GenerateSegmentPolygon(i);
		GenerateFatLine(i);

	}

	//Logic Idea -
	//1. For each Curve, for each segment, crate a bounding box with the curveID and bounds. Use a fitted polygon to estimate its bounding box
	//2. Then, for each curve, loop over find if any bounding box intersects another bounding box.
	//3. If here is an overlap - find where each segment polynomial returns the same value ** obviously hard


	//Trash Attempt use keypoints to make intersections
	FindKeyIntersection();
	DebugDrawIntersections();



}


//Temporary Test Function To find if with our splines how many intersections on key curve points exist
//This could be a cheap way to make intersections without needing complex maths
//We could also cheat, by looking at two curves, finding their straight line intersections, and if they are not parallel automatically create a point
void ACurveSandbox::FindKeyIntersection()
{
	

	int FirstSplineAKeyCount = Spline1->GetNumberOfSplinePoints();
	int SecondSplineKeyCount = Spline2->GetNumberOfSplinePoints();

	Intersections.Empty();
	//Debug Array for Points with Intersections
	


	for (int i = 0; i < FirstSplineAKeyCount; i++)
	{

		FVector PointA = Spline1->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);


		for (int j = 0; j < SecondSplineKeyCount; j++)
		{
			FVector PointB = Spline2->GetLocationAtSplinePoint(j, ESplineCoordinateSpace::World);

			if (FVector::Dist(PointA, PointB) <= 100.0f)
			{

				FIntersections IPoint;
				
				IPoint.IntersectPosition = PointA;
				IPoint.PointID.Add(i);
				IPoint.PointID.Add(j);
				IPoint.SplineReference.Add(Spline1);
				IPoint.SplineReference.Add(Spline2);

				Intersections.Add(IPoint);

			}

		}

	}



}


void ACurveSandbox::DebugDrawIntersections()
{

	if (Intersections.Num() > 0)
	{

		for (int i = 0; i < Intersections.Num(); i++)
		{
			DrawDebugSphere(GetWorld(), Intersections[i].IntersectPosition, 100.0f, 10, FColor::Green, true, -1.0f, 2.0f, 10.0f);


		}
	}

}



//Make a polygon that encapsulates the curvesegment
void ACurveSandbox::GenerateSegmentPolygon(int SegmentId)
{
	//Just do first segment on first spline for now
	TArray<FVector> Points;

	Points.Empty();

	FVector Point0 = Spline1->GetLocationAtSplinePoint(SegmentId, ESplineCoordinateSpace::World); //Start Point
	FVector Point1 = Spline1->GetLeaveTangentAtSplinePoint(SegmentId, ESplineCoordinateSpace::World); //Start Point Leave Tangent
	FVector Point2 = Spline1->GetLocationAtSplinePoint(SegmentId + 1, ESplineCoordinateSpace::World); //End Point
	FVector Point3 = Spline1->GetLeaveTangentAtSplinePoint(SegmentId + 1, ESplineCoordinateSpace::World); //End Point Arrive Tangent

	Points.Add(Point0);
	Points.Add(FMath::Lerp(Point0 + (Point1 * 0.5f), Point0, 0.5f));
	Points.Add(Point2);
	Points.Add(FMath::Lerp(Point2 + (-Point3 * 0.5f), Point2, 0.5f));


	for (int i = 0; i < Points.Num(); i++)
	{
		DrawDebugPoint(GetWorld(), Points[i], 20.0f, JunctionColorCodes[i], true, -1.0f, 10);
	}

	//Points
	TArray<FVector> BoundingBoxCorners;
	BoundingBoxCorners.Empty();

	TArray<float> Xcomponents;
	TArray<float> Ycomponents;

	float MaxX = 0.0f;
	float MaxY = 0.0f;
	float MinX = 0.0f;
	float MinY = 0.0f;


	//Go through each point and get min/max X and Y
	for (int i = 0; i < 4; i++)
	{
		FVector point = Points[i];

		Xcomponents.Add(point.X);
		Ycomponents.Add(point.Y);
	}

	MaxX = FGenericPlatformMath::Max(Xcomponents);
	MinX = FGenericPlatformMath::Min(Xcomponents);
	MaxY = FGenericPlatformMath::Max(Ycomponents);
	MinY = FGenericPlatformMath::Min(Ycomponents);


	TArray<FVector> BBoxPoint = {
		FVector(MaxX,MaxY,0.0f),
		FVector(MinX,MaxY,0.0f),
		FVector(MinX,MinY,0.0f),
		FVector(MaxX,MinY,0.0f)
	};



	FVector Center = (BBoxPoint[0] + BBoxPoint[1] + BBoxPoint[2] + BBoxPoint[3]) * 0.25f;
	FVector Extents = FVector((MaxX - MinX) * 0.5f, (MaxY - MinY) * 0.5, 1.0f);

	DrawDebugBox(GetWorld(), Center, Extents, FColor::Red, true, -1.0f, 2, 1.0f);

}


void ACurveSandbox::GenerateFatLine(int SegmentId)
{
	
	//Define LineN
	FVector Point0 = Spline1->GetLocationAtSplinePoint(SegmentId, ESplineCoordinateSpace::World); //Start Point
	FVector Point1 = Spline1->GetLocationAtSplinePoint(SegmentId + 1, ESplineCoordinateSpace::World); //Start Point


	//LineN
	FVector LineN = Point0 - Point1;

	DrawDebugLine(GetWorld(), Point0, Point1, FColor::Blue, true, -1.0f, 2, 1.0f);


	
	
	
}
	




// Called when the game starts or when spawned
void ACurveSandbox::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACurveSandbox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

