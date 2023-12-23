// Fill out your copyright notice in the Description page of Project Settings.


#include "RoadSurface.h"
#include "ProceduralMeshComponent.h"

// Sets default values
ARoadSurface::ARoadSurface()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RoadSurface = CreateDefaultSubobject<UProceduralMeshComponent>("Road");
	RoadSurface->SetupAttachment(GetRootComponent());

	CenterSpline = CreateDefaultSubobject<USplineComponent>("CenterSpline");
	CenterSpline->SetupAttachment(RoadSurface);

}


void ARoadSurface::OnConstruction(const FTransform& RootTransform)
{
	Super::OnConstruction(RootTransform);

	//1. Make List of all WayPoints (UserInput atm)
	//2. Make Edge Vertices (Generate Mesh Points)
	//3. Set them as vertices

	int pointCount = RoadPoints.Num();

	//UE_LOG(LogTemp, Warning, TEXT("Road Point Count: %d"), pointCount);

	//Run our Functions to Create our Data for our Spline
	//PointsAlongSpline(0.0, CenterSpline->GetSplineLength());
	//GenerateMeshPoints();
	//GenerateTriangles();



	if (EditorMode == true)
	{
		BuildRoad();
	}
	//Create Final Mesh 
	//RoadSurface->CreateMeshSection(0, VertexPositions, TriangleIndices, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);



	if (ProjectSpline == true)
	{
	
		ProjectSpline = false;
		ProjectSplinePoints();

	}




}

//Trigger this rebuild 
void ARoadSurface::BuildRoad()
{




	//Check Intersect Points

	if (EditorMode == true)
	{
		PointsAlongSpline(0.0f, CenterSpline->GetSplineLength());
		GenerateMeshPoints();
		GenerateTriangles();


		//Create Final Mesh 
		RoadSurface->CreateMeshSection(0, VertexPositions, TriangleIndices, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);

	}
	else
	{ 

	/*

		//Calculate how many subpieces we need
		int RoadSubSectionCount;



		if (IntersectPoints.IsEmpty())
		{
			RoadSubSectionCount = 1;
			IntersectPoints[0] = 0.0f;
			IntersectPoints[0] = CenterSpline->GetSplineLength();
		}
		else
		{
			RoadSubSectionCount = IntersectPoints.Num() - 1;
		}

		IntersectPoints.Sort();

		//Run our mesh generation over each road section
		for (int i = 0; i < RoadSubSectionCount; i += 2)
		{

			//Generate our new Set of Spline Points

			const float StartPosition = IntersectPoints[i];
			float EndPosition = IntersectPoints[i];

			//if (i >= IntersectPoints.Num())
			//{
			//	EndPosition = CenterSpline->GetSplineLength();

			//}
			//else
			//{
			//	EndPosition = IntersectPoints[i+1];
			//}


			PointsAlongSpline(StartPosition, EndPosition);
			GenerateMeshPoints();
			GenerateTriangles();


			//Create Final Mesh 
			RoadSurface->CreateMeshSection(i, VertexPositions, TriangleIndices, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);


			//Notes: Get Start Position from Array
			//Get End Position from Array
			//Feed into Mesh Generation as Start and End
			//Generated Positions along the curve
			//Generate Vertex Positions
			//Feed to Surface
			//Create Mesh Subsection

		}

	*/


	}

}



//Genate Array of all our Points here
void ARoadSurface::GenerateMeshPoints()
{
	//Clear All Vertices
	VertexPositions.Empty();

	int pointCount = SplinePositions.Num();

	//For each waypoint go through and create a vertex either side
	//We will expand this later into it's own set of functions of build dense grids
	for (int i = 0; i < pointCount; i++)
	{
		//Get Current Position - Build a position to the left and to the right and add it to array
		FVector wayPointPosition = SplinePositions[i];
		FVector leftPosition =  wayPointPosition + (-RoadPointTangent[i] * RoadWidth);
		FVector rightPosition = wayPointPosition + (RoadPointTangent[i] * RoadWidth);

		float positionAlongWidth = 1.0 / (WidthResolution + 3);

		//Add Points Based on Resolution
		for (int j = 0; j < WidthResolution + 4; j++)
		{

			float t = positionAlongWidth * j;
			FVector incrementalPosition = FMath::Lerp(leftPosition, rightPosition, t);
			VertexPositions.Add(incrementalPosition);
		}
	}
	return;
}

//Generate all our Triangles Indices here
void ARoadSurface::GenerateTriangles()
{

	TriangleIndices.Empty();

	//We need to go through all our vertex positions and generate triangles - its a bit anoying we need to use a formula to always generate clean quads
	//Right now - we generate one quad per road point - so each roadpoint requires SIX indices = roadPoints.Num * 6

	int vertexCount = VertexPositions.Num();
	int pointCount = SplinePositions.Num();  // RoadPoints.Num();

	int widthPoints = WidthResolution + 4;

	int rootIndex = 0;

	//Each Step Along Road 
	for (int i = 0; i < pointCount; i++)
	{

		for (int j = 0; j < (widthPoints - 1); j++)
		{
			//FIRST TRIANGLE
			int first = (rootIndex + j);
			int second = (rootIndex + j) + 1;
			int third = (rootIndex + j) + widthPoints;

			TriangleIndices.Add(first);
			TriangleIndices.Add(second);
			TriangleIndices.Add(third);
		
			//SECOND TRIANGLE
			int fourth = (rootIndex + j) + 1;
			int fifth = (rootIndex + j) + (widthPoints + 1);
			int sixth = (rootIndex + j) + widthPoints;

			TriangleIndices.Add(fourth);
			TriangleIndices.Add(fifth);
			TriangleIndices.Add(sixth);
		}
		rootIndex = i * widthPoints;
	}
	return;
}


//Get Points Along Spline
void ARoadSurface::PointsAlongSpline(float StartDistance, float EndDistance)
{

	FVector RoadWorldSpaceOffset = this->GetTransform().GetLocation();

	//Clear our Arrays
	SplinePositions.Empty();
	RoadPointNormal.Empty();
	RoadPointTangent.Empty();

	//Spline Resolution should be Resolution over Distance
	float SplineLength = CenterSpline->GetSplineLength();
	//Number of Points we need to make
	int SplineDensity = int(SplineLength / LengthResolution);
	//Distance Between Each Point
	float PartDistance = (SplineLength / SplineDensity);

	UE_LOG(LogTemp, Warning, TEXT("Road Point Count: %d"), SplineDensity);

	if (SplineDensity < 1.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROAD HAS NO POINTS"));
		return;
	}


	for (int i = 0; i < (SplineDensity + 1); i++)
	{

		float t = StartDistance + (PartDistance * i);

		if (t > EndDistance)
		{
			break;
		}

		FVector WorldLocation = CenterSpline->GetWorldLocationAtDistanceAlongSpline(t);
		FVector WorldNormal = CenterSpline->GetWorldDirectionAtDistanceAlongSpline(t);
		FVector WorldTangent = CenterSpline->GetRightVectorAtDistanceAlongSpline(t, ESplineCoordinateSpace::World);

		SplinePositions.Add(WorldLocation - RoadWorldSpaceOffset);
		RoadPointNormal.Add(WorldNormal);
		RoadPointTangent.Add(WorldTangent);

	}

	//Add Final Point Here

}

//Project Spline Points to Terrain
void ARoadSurface::ProjectSplinePoints()
{

	if ((CenterSpline == nullptr) || (CenterSpline->GetNumberOfSplinePoints() == 0))
	{
		return;
	}


	//For Each Point in our Spline, project it down until it hits something
	for (int i = 0; i < CenterSpline->GetNumberOfSplinePoints(); i++)
	{
		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		FCollisionObjectQueryParams ObjectParams;

		TEnumAsByte<ECollisionChannel> TraceChannelProperty = ECC_WorldStatic;

		FVector StartLocation = CenterSpline->GetWorldLocationAtSplinePoint(i) - FVector(0,0,-50.0f);
		FVector EndLocation = StartLocation + (FVector(0, 0, -1) * 5000.0f);


		bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult,StartLocation, EndLocation, ObjectParams, CollisionParams);

		//if(Hi)



	}




}



// Called when the game starts or when spawned
void ARoadSurface::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARoadSurface::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

