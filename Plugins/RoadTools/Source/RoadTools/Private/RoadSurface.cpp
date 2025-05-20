// Fill out your copyright notice in the Description page of Project Settings.


#include "RoadSurface.h"
#include "ProceduralMeshComponent.h"
#include "Landscape.h"
#include "PlotGenerator.h"
#include "LaneSpline.h"

// Sets default values
ARoadSurface::ARoadSurface()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RoadSurface = CreateDefaultSubobject<UProceduralMeshComponent>("Road");
	RoadSurface->SetupAttachment(GetRootComponent());

	LaneMarkingSurface = CreateDefaultSubobject<UProceduralMeshComponent>("LaneMarkings");
	LaneMarkingSurface->SetupAttachment(GetRootComponent());

	CenterSpline = CreateDefaultSubobject<USplineComponent>("CenterSpline");
	CenterSpline->SetupAttachment(RoadSurface);

}


void ARoadSurface::OnConstruction(const FTransform& RootTransform)
{
	Super::OnConstruction(RootTransform);

	FlushPersistentDebugLines(GetWorld());
	//1. Make List of all WayPoints (UserInput atm)
	//2. Make Edge Vertices (Generate Mesh Points)
	//3. Set them as vertices

	int pointCount = RoadPoints.Num();

	if (EditorMode == true && UseAdvancedRoadDesigner == false)
	{
		BuildRoad();
	}
	//Create Final Mesh 
	//RoadSurface->CreateMeshSection(0, VertexPositions, TriangleIndices, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);

	//Use Advanced Road Designer
	if (UseAdvancedRoadDesigner == true)
	{
		//FlushPersistentDebugLines(GetWorld());
		DrawDebugPoint(GetWorld(), FVector(0,0,0), 10.0f, FColor::Blue, true, -1.0f, 1);

		RoadActorLocation = this->GetActorLocation();

		//For Each Lane, create a mesh
		int TotalLaneCount = LeftLanes.Num();

		//Clear All Lanes
		RoadSurface->ClearAllMeshSections();
		LaneMarkingSurface->ClearAllMeshSections();


		for (int i = 0; i < TotalLaneCount; i++)
		{
			BuildAdvancedRoad(LeftLanes,i, 0);
		}

		//For Each Lane, create a mesh
		TotalLaneCount = RightLanes.Num();

		for (int i = 0; i < TotalLaneCount; i++)
		{
			BuildAdvancedRoad(RightLanes,i, LeftLanes.Num());
		}


	}


	//DebugDrawVertices();

	if (ProjectSpline == true)
	{
		FlushPersistentDebugLines(GetWorld());
		ProjectSpline = false;
		ProjectSplinePoints();
	}



	//Update Plots If Enabled
	if (GeneratePlots == true)
	{
		UPlotGenerator* PlotComponent = Cast<UPlotGenerator>(GetComponentByClass(UPlotGenerator::StaticClass()));
		if (PlotComponent)
		{
			PlotComponent->GeneratePlotAreas(SplinePointData);
		}
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

void ARoadSurface::BuildAdvancedRoad(TArray<FLaneData> Lanes,int LaneIndex, int offset)
{

	//Make Base Road Mesh each lane is a mesh section to allow for multiple materials
	PointsAlongSpline(0.0f, CenterSpline->GetSplineLength());
	GenerateAdvancedMeshPoints(Lanes, LaneIndex);
	GenerateAdvancedTriangles();

	RoadSurface->CreateMeshSection(LaneIndex + offset, AdvancedVertexPositions, TriangleIndices, TArray<FVector>(), AdvancedVertexUV, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	RoadSurface->SetMaterial(LaneIndex + offset, Lanes[LaneIndex].Material);
	RoadSurface->CastShadow = false;

	//Create Road Markings

	if (CreateLaneMarkings == true)
	{


		//For Each Generate Mesh Points for Lanes Markings
		//Generate Triangles
		//Pass to LaneMarkings Proc Mesh Component
		if (Lanes[LaneIndex].LaneMarkings.Num() == 0)
		{
			return;
		}

		GenerateLaneMarkings(Lanes, LaneIndex);
		//Create Lane Markings Mesh
		LaneMarkingSurface->CreateMeshSection(LaneIndex + offset, LaneMarkingVertices, TriangleIndices, TArray<FVector>(), LaneMarkingUVs,TArray<FColor>(), TArray<FProcMeshTangent>(), true);
		LaneMarkingSurface->SetMaterial(LaneIndex + offset, Lanes[LaneIndex].LaneMarkings[0].MarkingMaterial);
		LaneMarkingSurface->CastShadow = false;
	
	
	}
}


//Make Lane Markings
void ARoadSurface::GenerateLaneMarkings(TArray<FLaneData> Lanes, int LaneIndex)
{

	//
	const int PointCount = SplinePointData.Num();
	SplineLength = CenterSpline->GetSplineLength();
	const float UVIncrement = (SplineLength / 100.0f) / PointCount;

	LaneMarkingVertices.Empty();
	LaneMarkingUVs.Empty();

	for (int i = 0; i < PointCount; i++)
	{


		TArray<float> Widths; //All Accumilated Widths
		Widths.Add(0);		  //Center Line
		float SumWidths = 0.0;

		//Make an array of accumilated point offsets 
		for (int j = 0; j < Lanes.Num(); j++)
		{
			SumWidths = SumWidths + Lanes[j].LaneWidth;
			Widths.Add(SumWidths);

		}

		//Get First Point  --  INCREMENT EACH MARKING INDEX
		float P0 = Widths[LaneIndex] + Lanes[LaneIndex].LaneMarkings[0].MarkingOffset; //Lane Start Point
		float P1 = Widths[LaneIndex] + Lanes[LaneIndex].LaneMarkings[0].MarkingOffset + Lanes[LaneIndex].LaneMarkings[0].MarkingWidth;


		FVector CenterPointPosition = SplinePointData[i].Location;
		FVector NewP0 = (CenterPointPosition + (SplinePointData[i].RightVector * P0)) + FVector(0.0f,0.0f,0.1f);
		FVector NewP1 = (CenterPointPosition + (SplinePointData[i].RightVector * P1)) + FVector(0.0f, 0.0f, 0.1f);

		LaneMarkingVertices.Add(NewP0);
		LaneMarkingVertices.Add(NewP1);


		//UV Calc
		float PositionAlongSpline = UVIncrement * i;

		FVector2D UV0 = FVector2D(PositionAlongSpline, 1.0f);
		LaneMarkingUVs.Add((UV0 * Lanes[LaneIndex].LaneMarkings[0].UVTiling) + Lanes[LaneIndex].LaneMarkings[0].UVOffset);
		FVector2D UV1 = FVector2D(PositionAlongSpline, 0.0f);
		LaneMarkingUVs.Add((UV1 * Lanes[LaneIndex].LaneMarkings[0].UVTiling) + Lanes[LaneIndex].LaneMarkings[0].UVOffset);



	}



}



//Overrite this with GenerateMeshPoints
void ARoadSurface::GenerateAdvancedMeshPoints(TArray<FLaneData> Lanes, int LaneIndex)
{

	//VertexPositions.Empty();

	const int PointCount = SplinePointData.Num();
	const int TotalLaneCount = Lanes.Num();

	//Get Length of Spline for UVs
	SplineLength = CenterSpline->GetSplineLength();
	const float UVIncrement = (SplineLength / 100.0f) / PointCount;

	UE_LOG(LogTemp, Warning, TEXT("RoadSurface: GenerateAdvancedMeshPoints. SplinePointData is %i"), SplinePointData.Num());

	AdvancedVertexPositions.Empty();
	AdvancedVertexUV.Empty();

	//For Each Point Along our Main Spline
	for (int i = 0; i < PointCount; i++)
	{

		//For Each Spine Index Point
		//Get our Lane Index
		//for our index count, accumulate the widths we need
		//start = lanes[index - 1]


		TArray<float> Widths; //All Accumilated Widths
		Widths.Add(0);		  //Center Line
		float SumWidths = 0.0;

		//Make an array of accumilated point offsets 
		for (int j = 0; j < Lanes.Num(); j++)
		{
			SumWidths = SumWidths + Lanes[j].LaneWidth;
			Widths.Add(SumWidths);

		}

		//Get First Point
		float P0 = Widths[LaneIndex];
		float P1 = Widths[LaneIndex + 1];

		float StartPoint = FMath::Min(P0, P1);
		float EndPoint	 = FMath::Max(P0, P1);

		// --
		// Insert Lerped Points here for lane resolution


		//Resolve our WorldSpace Data
		FVector CenterPointPosition = SplinePointData[i].Location;
		FVector NewP0 = (CenterPointPosition + (SplinePointData[i].RightVector * StartPoint)) - RoadActorLocation;
		FVector NewP1 = (CenterPointPosition + (SplinePointData[i].RightVector * EndPoint)) - RoadActorLocation;

		AdvancedVertexPositions.Add(NewP0);
		AdvancedVertexPositions.Add(NewP1);


		//For Each Vertex we add, add a uv. Left = y - 1 Right - y = 0. X is Position Along our Spline
		float PositionAlongSpline = UVIncrement * i;

		FVector2D UV0 = FVector2D(PositionAlongSpline, 1.0f);
		AdvancedVertexUV.Add((UV0 * Lanes[LaneIndex].UVTiling) + Lanes[LaneIndex].UVOffset);
		FVector2D UV1 = FVector2D(PositionAlongSpline, 0.0f);
		AdvancedVertexUV.Add((UV1 * Lanes[LaneIndex].UVTiling) + Lanes[LaneIndex].UVOffset);

		// End of Loop
	}

	ProfileVertexCount = LeftLanes.Num() + RightLanes.Num();

	UE_LOG(LogTemp, Warning, TEXT("RoadSurface: GenerateAdvancedMeshPoints. Vertex Count is %i"),AdvancedVertexPositions.Num());

	
	//Editor Debug


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

void ARoadSurface::GenerateAdvancedTriangles()
{
	TriangleIndices.Empty();

	//We need to go through all our vertex positions and generate triangles - its a bit anoying we need to use a formula to always generate clean quads
	//Right now - we generate one quad per road point - so each roadpoint requires SIX indices = roadPoints.Num * 6

	int vertexCount = VertexPositions.Num();
	int pointCount = SplinePositions.Num();  // RoadPoints.Num();

	int widthPoints = 2;

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
	SplinePositions.Empty();  //V1
	RoadPointNormal.Empty();  //V1
	RoadPointTangent.Empty(); //V1

	SplinePointData.Empty();  //V2

	//Spline Resolution should be Resolution over Distance
	 SplineLength = CenterSpline->GetSplineLength();
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


		//V2 
		FSampledSplinePoint NewPoint;
		NewPoint.Location = WorldLocation;
		NewPoint.RightVector = WorldTangent;
		NewPoint.Normal = WorldNormal;

		SplinePointData.Add(NewPoint);
		
		// END V2


	}

	//Add Final Point Here

}

void ARoadSurface::DebugDrawVertices()
{

	if (AdvancedVertexPositions.Num() != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RoadSurface: DebugDrawVertices Drawing Verts"));

		for (int i = 0; i < AdvancedVertexPositions.Num(); i++)
		{
			DrawDebugPoint(GetWorld(), AdvancedVertexPositions[i], 20.0f, FColor::Blue, true, -1.0f, 10);
		}
	}

}

void ARoadSurface::BuildAndUpdateLaneSplines()
{
	UE_LOG(LogTemp, Log, TEXT("Generated Lane Splines for traffic"));

	//Move this to a function later
	const int LaneSplinesToGenerate = LeftLanes.Num() + RightLanes.Num();
	const int BoundLaneSplines = GeneratedLaneSplines.Num();
	
	const int GeneratedResolution = CenterSpline->GetSplineLength() / DistanceBetweenPoints; //Will Make One Point Per Meter

	TArray<TSoftObjectPtr<ALaneSpline>> NewlyGeneratedLaneSplines;
	
	//For now, just delete/Remove all old splines, saves some hassle
	for (TSoftObjectPtr<ALaneSpline> CurrentLaneSpline : GeneratedLaneSplines)
	{
		TObjectPtr<ALaneSpline> LoadedSpline = CurrentLaneSpline.LoadSynchronous();

		if (LoadedSpline)
		{
			LoadedSpline->Destroy();
		}
	}

	GeneratedLaneSplines.Empty();

	if (LeftLanes.Num() > 0)
	{
		for (int i = 0; i < LeftLanes.Num(); i++)
		{
			//Create a whole new spline
			TArray<FSplinePoint> CurrentLanePoints = CreateLanePoints(GeneratedResolution, i, LeftLanes[i], LeftLanes, 0);

			if (!CurrentLanePoints.IsEmpty())
			{
				//Spawn A Lane Actor
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(this->GetActorLocation());
				FActorSpawnParameters SpawnParams;

				TObjectPtr<ALaneSpline> NewLaneSpline = GetWorld()->SpawnActor<ALaneSpline>(SpawnParams);

				if (NewLaneSpline)
				{
					TObjectPtr<USplineComponent> SplineComponent = NewLaneSpline->LaneSpline;

					NewLaneSpline->SetActorLocation(SpawnTransform.GetLocation());
					NewLaneSpline->LaneDirection = 0;

					SplineComponent->ClearSplinePoints();
					SplineComponent->AddPoints(CurrentLanePoints, true);

					GeneratedLaneSplines.Add(NewLaneSpline);
				}
			}
		}
	}

	if (RightLanes.Num() > 0)
	{
		for (int i = 0; i < RightLanes.Num(); i++)
		{
			//Create a whole new spline
			TArray<FSplinePoint> CurrentLanePoints = CreateLanePoints(GeneratedResolution, i, RightLanes[i], RightLanes, 1);

			if (!CurrentLanePoints.IsEmpty())
			{
				//Spawn A Lane Actor
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(this->GetActorLocation());
				FActorSpawnParameters SpawnParams;

				TObjectPtr<ALaneSpline> NewLaneSpline = GetWorld()->SpawnActor<ALaneSpline>(SpawnParams);

				if (NewLaneSpline)
				{
					TObjectPtr<USplineComponent> SplineComponent = NewLaneSpline->LaneSpline;

					NewLaneSpline->SetActorLocation(SpawnTransform.GetLocation());
					NewLaneSpline->LaneDirection = 1;
					SplineComponent->ClearSplinePoints();
					SplineComponent->AddPoints(CurrentLanePoints, true);
					
					GeneratedLaneSplines.Add(NewLaneSpline);
				}
			}
		}
	}
}

TArray<FSplinePoint> ARoadSurface::CreateLanePoints(const int InResolution, const int LaneID, const FLaneData InLane, const TArray<FLaneData> InLaneArray, const int LaneDirection)
{
	TArray<FSplinePoint> LanePoints;

	const float SplineLength = CenterSpline->GetSplineLength();
	float IncrementDistance = SplineLength / InResolution;
	float DistanceAlongSpline = 0.0f;


	for (int i = 0; i < InResolution + 1; i++)
	{
		float TotalOffsetForLane = -InLaneArray[0].LaneWidth * 0.5f;

		//Get Offset At Lane Cross Section
		for (int j = 0; j < LaneID + 1; j++)
		{
			TotalOffsetForLane += (InLaneArray[j].LaneWidth);
			UE_LOG(LogTemp, Log, TEXT("Lane Width at index %i, is: %f"), j, TotalOffsetForLane);
		}

		//-- Flip Spline Sample 
		if (LaneDirection == 0)
		{
			//Forward Sample
			DistanceAlongSpline = FMath::Clamp(IncrementDistance * i, 0, CenterSpline->GetSplineLength());
		}
		else
		{
			//Backward Direction
			DistanceAlongSpline = SplineLength - FMath::Clamp(IncrementDistance * i, 0, CenterSpline->GetSplineLength());
		}

		//Get Offset Position
		FVector OffsetPoint = CenterSpline->GetLocationAtDistanceAlongSpline(FMath::Clamp(DistanceAlongSpline, 0, CenterSpline->GetSplineLength()), ESplineCoordinateSpace::World);
		FVector OffsetVector = CenterSpline->GetRightVectorAtDistanceAlongSpline(FMath::Clamp(DistanceAlongSpline, 0, CenterSpline->GetSplineLength()), ESplineCoordinateSpace::World);

		//Offset our Point
		FSplinePoint NewPoint;

		NewPoint.Position = (OffsetPoint - this->GetActorLocation()) + (OffsetVector * TotalOffsetForLane);
		NewPoint.InputKey = i;
		NewPoint.Scale = FVector(1, 1, 1);

		LanePoints.Add(NewPoint);
	}

	return LanePoints;

}


void ARoadSurface::CreateLaneSpline(const TArray<FVector> InSplinePoints)
{

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
		FHitResult UpHitResult;
		FHitResult DownHitResult;
		FCollisionQueryParams CollisionParams;
		FCollisionObjectQueryParams ObjectParams;

		TEnumAsByte<ECollisionChannel> TraceChannelProperty = ECC_WorldStatic;

		FVector StartLocation = CenterSpline->GetWorldLocationAtSplinePoint(i) - FVector(0,0,-1.0f);
		FVector DownLocation = StartLocation + (FVector(0, 0, -1) * 10000.0f);
		FVector UpLocation   = StartLocation + (FVector(0, 0, 1) * 10000.0f);

		//bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStartPosition, EndLocation, ECC_WorldDynamic, CollisionParms);

		bool bHitDown = GetWorld()->LineTraceSingleByObjectType(DownHitResult,StartLocation, DownLocation, ECC_WorldStatic, CollisionParams);
		bool bHitUp = GetWorld()->LineTraceSingleByObjectType(UpHitResult, StartLocation, UpLocation, ECC_WorldStatic, CollisionParams);

		DrawDebugLine(GetWorld(), StartLocation, DownLocation, FColor::Red, true, 10.0f, 10, 10.0f);

		FVector ResultLocation = CenterSpline->GetWorldLocationAtSplinePoint(i);

		
		if (bHitDown)
		{
			AActor* HitActor =  DownHitResult.GetActor();

			ResultLocation = DownHitResult.Location;

			if (HitActor)
			{
					ResultLocation = DownHitResult.Location;
			}


			DrawDebugSphere(GetWorld(), ResultLocation, 10.0f, 32, FColor::Green, true, 2.0f, 1, 1.0f);

			UE_LOG(LogTemp, Warning, TEXT("RoadSurface: Project To Terrain, %s"), *ResultLocation.ToString());
			CenterSpline->SetLocationAtSplinePoint(i, ResultLocation + FVector(0.0f,0.0f,0.2f), ESplineCoordinateSpace::World, true);

		}
		


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

