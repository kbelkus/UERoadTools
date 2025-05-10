// Fill out your copyright notice in the Description page of Project Settings.


#include "JunctionSurface.h"
#include "ProceduralMeshComponent.h"
#include "MathHelperFunctions.h"
#include "DrawDebugHelpers.h"
#include "Math/Vector.h"
#include "LaneSpline.h"

DECLARE_STATS_GROUP(TEXT("EditorRoadTools"), STATGROUP_EditorRoadTools, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("BuildJunction"), STAT_BuildJunction, STATGROUP_EditorRoadTools);


// Sets default values
AJunctionSurface::AJunctionSurface()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	JunctionSurface = CreateDefaultSubobject<UProceduralMeshComponent>("RoadSurface");
	JunctionSurface->SetupAttachment(GetRootComponent());

	JunctionCenterSurface = CreateDefaultSubobject<UProceduralMeshComponent>("JunctionCenter");
	JunctionCenterSurface->SetupAttachment(GetRootComponent());

	LaneMarkingsSurface = CreateDefaultSubobject<UProceduralMeshComponent>("LaneMarkingsSurface");
	LaneMarkingsSurface->SetupAttachment(GetRootComponent());

	//TurningLanesSurface = CreateDefaultSubobject<UProceduralMeshComponent>("TurningLanesSurface");
	//TurningLanesSurface->SetupAttachment(GetRootComponent());
}

void AJunctionSurface::BuildJunction()
{
		//GenerateJunctionPoints();
		DrawJunctionShape();
		DrawJunctionVolume();
}

void AJunctionSurface::OnConstruction(const FTransform& RootTransform)
{

	Super::OnConstruction(RootTransform);
	{
		SCOPE_CYCLE_COUNTER(STAT_BuildJunction);

	//Mode for when artists want to manually create a junction 
		if (ManualEditMode == true)
		{
			FlushPersistentDebugLines(GetWorld());

			JunctionSurface->ClearAllMeshSections();
			JunctionCenterSurface->ClearAllMeshSections();
			LaneMarkingsSurface->ClearAllMeshSections();
			//TurningLanesSurface->ClearAllMeshSections();

			ManualEditInitialiseJunction();
			//ManualEditDrawJunctionShape();

			int MeshIndex = 0;

			JunctionCapPoints.Empty();
			JunctionCenterBoundaryPoints.Empty(); //NO NEEDED?
			JunctionIDSortedPoints.Empty();
			JunctionCenterLineEndPoint.Empty();

			//For Each Junction
			for (int i = 0; i < JunctionPoints.Num(); i++)
			{
				//Junction Specific Data
				TArray<FCapPoints> CurrentJunctionPoints;
				TArray<FCapPoints> TurningLaneCenterPoints;

				//LEFT LANES
				if (JunctionPoints[i].LeftLanes.Num() != 0)
				{

					ManualEditCreateLaneBoundaries(JunctionPoints[i].LeftLanes, JunctionPoints[i].CenterLinePoints);

					float LaneLength = FVector::Distance(JunctionPoints[i].Location, JunctionPoints[i].EndLocation);

					for (int j = 0; j < JunctionPoints[i].LeftLanes.Num(); j++)
					{
						//Create Geo for Each Junction
						ManualEditCreateLaneVertices(j, JunctionPoints[i].CenterLinePoints.Num(), JunctionPoints[i].LeftLanes.Num(), LaneLength, JunctionPoints[i].LeftLanes[j].UVTiling, JunctionPoints[i].LeftLanes[j].UVOffset);
						ManualEditCreateLaneTriangles(JunctionPoints[i].CenterLinePoints.Num(), true);

						//DrawVertices(VertexPositions);
						JunctionSurface->CreateMeshSection(MeshIndex, VertexPositions, TriangleIndices, TArray<FVector>(), UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
						JunctionSurface->SetMaterial(MeshIndex, JunctionPoints[i].LeftLanes[j].Material);
						JunctionSurface->bCastDynamicShadow = false;

						MeshIndex = MeshIndex + 1;

						//FVector TTEmpLocation = JunctionPoints[i].CenterLinePoints[JunctionPoints[i].CenterLinePoints.Num() - 1].Location;
						//DrawDebugPoint(GetWorld(), TTEmpLocation, 20.0f, FColor::Orange, true, -1.0f, 5);

						//Lane Markings for this Junction
						if (JunctionPoints[i].LeftLanes[j].LaneMarkings.Num() != 0)
						{
							ManualEditCreateLaneMarkingVertices(j, JunctionPoints[i].CenterLinePoints.Num(), JunctionPoints[i].LeftLanes.Num(), LaneLength, JunctionPoints[i].LeftLanes[j].LaneMarkings[0].UVTiling, JunctionPoints[i].LeftLanes[j].LaneMarkings[0].UVOffset, JunctionPoints[i].LeftLanes[j].LaneMarkings[0].MarkingWidth, JunctionPoints[i].LeftLanes[j].LaneMarkings[0].MarkingOffset);
							ManualEditCreateLaneTriangles(JunctionPoints[i].CenterLinePoints.Num(), false);

							LaneMarkingsSurface->CreateMeshSection(MeshIndex, LaneMarkingVertices, TriangleIndices, TArray<FVector>(), UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
							LaneMarkingsSurface->SetMaterial(MeshIndex, JunctionPoints[i].LeftLanes[j].LaneMarkings[0].MarkingMaterial);
							LaneMarkingsSurface->bCastDynamicShadow = false;

						}
					}

					//Cap Points
					TArray<FCapPoints> LeftHandPoints;
					int BoundaryPointsCount = LaneBoundaryPositions.Num();
					//UE_LOG(LogTemp, Warning, TEXT("JunctionSurface Lane Left Boundary Point Count Junction then Count: %i, %i"), i,BoundaryPointsCount);

					int LaneCount = JunctionPoints[i].LeftLanes.Num();

					//For our Lane Count, get the last point and add it to the array
					for (int p = 0; p < JunctionPoints[i].LeftLanes.Num(); p++)
					{
						int PointType = 0;

						if (p == 0)
						{
							PointType = -1;
						}

						int PointID = (BoundaryPointsCount - 1) - p;

						FCapPoints NewPoint;

						NewPoint.JunctionID = i;
						NewPoint.Location = LaneBoundaryPositions[PointID];
						NewPoint.PointType = PointType;
						NewPoint.ForwardVector = JunctionPoints[i].ForwardVector; //CHANGE THIS LATER IF THE JUNCTION CAN HAVE A CURVED ROAD
						NewPoint.RightVector = FVector::CrossProduct(JunctionPoints[i].ForwardVector, FVector(0, 0, 1));
						NewPoint.AngleFromCenter = 0.0f;
						NewPoint.PointID = p;
						NewPoint.OffsetDistance = 0.0f;
						NewPoint.UValue = 0.0f;
						NewPoint.LaneDirection = 0;
						NewPoint.TurningRule = JunctionPoints[i].LeftLanes[p].TurningRule;
						NewPoint.RoadType = JunctionPoints[i].LeftLanes[p].RoadType;
						//DrawDebugPoint(GetWorld(), LaneBoundaryPositions[PointID], 20.0f, JunctionColorCodes[p], true, -1.0f, 5);

						LeftHandPoints.Add(NewPoint);

					}

					JunctionCapPoints.Append(LeftHandPoints);

					//Append our Center Point here
					FCapPoints CenterPoint;

					CenterPoint.JunctionID = i;
					CenterPoint.Location = JunctionPoints[i].CenterLinePoints[JunctionPoints[i].CenterLinePoints.Num() - 1].Location;
					CenterPoint.ForwardVector = JunctionPoints[i].CenterLinePoints[JunctionPoints[i].CenterLinePoints.Num() - 1].ForwardVector;
					CenterPoint.PointType = 2;
					CenterPoint.LaneDirection = 0;
					CenterPoint.RoadType = JunctionPoints[i].LeftLanes[0].RoadType;
					CenterPoint.TurningRule = JunctionPoints[i].LeftLanes[0].TurningRule;
					CenterPoint.RightVector = FVector::CrossProduct(JunctionPoints[i].ForwardVector, FVector(0, 0, 1));

					JunctionCapPoints.Add(CenterPoint);

					CurrentJunctionPoints.Append(LeftHandPoints);
					CurrentJunctionPoints.Add(CenterPoint);
					JunctionCenterLineEndPoint.Add(CenterPoint);
				}

				//RIGHT LANES
				if (JunctionPoints[i].RightLanes.Num() != 0)
				{
					ManualEditCreateLaneBoundaries(JunctionPoints[i].RightLanes, JunctionPoints[i].CenterLinePoints);
					float LaneLength = FVector::Distance(JunctionPoints[i].Location, JunctionPoints[i].EndLocation);

					for (int j = 0; j < JunctionPoints[i].RightLanes.Num(); j++)
					{

						//Create Geo for Each Junction
						ManualEditCreateLaneVertices(j, JunctionPoints[i].CenterLinePoints.Num(), JunctionPoints[i].RightLanes.Num(), LaneLength, JunctionPoints[i].RightLanes[j].UVTiling, JunctionPoints[i].RightLanes[j].UVOffset);
						ManualEditCreateLaneTriangles(JunctionPoints[i].CenterLinePoints.Num(), false);

						//DrawVertices(VertexPositions);
						JunctionSurface->CreateMeshSection(MeshIndex, VertexPositions, TriangleIndices, TArray<FVector>(), UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
						JunctionSurface->SetMaterial(MeshIndex, JunctionPoints[i].RightLanes[j].Material);
						JunctionSurface->bCastDynamicShadow = false;

						MeshIndex = MeshIndex + 1;


						//Lane Markings for this Junction
						if (JunctionPoints[i].RightLanes[j].LaneMarkings.Num() != 0)
						{

							ManualEditCreateLaneMarkingVertices(j, JunctionPoints[i].CenterLinePoints.Num(), JunctionPoints[i].RightLanes.Num(), LaneLength, JunctionPoints[i].RightLanes[j].LaneMarkings[0].UVTiling, JunctionPoints[i].RightLanes[j].LaneMarkings[0].UVOffset, JunctionPoints[i].RightLanes[j].LaneMarkings[0].MarkingWidth, JunctionPoints[i].RightLanes[j].LaneMarkings[0].MarkingOffset);
							ManualEditCreateLaneTriangles(JunctionPoints[i].CenterLinePoints.Num(), true);

							LaneMarkingsSurface->CreateMeshSection(MeshIndex, LaneMarkingVertices, TriangleIndices, TArray<FVector>(), UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
							LaneMarkingsSurface->SetMaterial(MeshIndex, JunctionPoints[i].RightLanes[j].LaneMarkings[0].MarkingMaterial);
							LaneMarkingsSurface->bCastDynamicShadow = false;

						}


					}

					//Cap Points
					TArray<FCapPoints> RightHandPoints;
					int BoundaryPointsCount = LaneBoundaryPositions.Num();
					//UE_LOG(LogTemp, Warning, TEXT("JunctionSurface Lane Left Boundary Point Count Junction then Count: %i, %i"), i, BoundaryPointsCount);

					int LaneCount = JunctionPoints[i].RightLanes.Num();

					//For our Lane Count, get the last point and add it to the array
					for (int p = 0; p < JunctionPoints[i].RightLanes.Num(); p++)
					{
						int PointType = 0;

						if (p == 0)
						{
							PointType = 1;
						}

						int PointID = (BoundaryPointsCount - 1) - p;

						FCapPoints NewPoint;

						NewPoint.JunctionID = i;
						NewPoint.Location = LaneBoundaryPositions[PointID];
						NewPoint.PointType = PointType;
						NewPoint.ForwardVector = JunctionPoints[i].ForwardVector; //CHANGE THIS LATER IF THE JUNCTION CAN HAVE A CURVED ROAD
						NewPoint.RightVector = FVector::CrossProduct(JunctionPoints[i].ForwardVector, FVector(0, 0, 1));
						NewPoint.AngleFromCenter = 0.0f;
						NewPoint.PointID = p;
						NewPoint.OffsetDistance = 0.0f;
						NewPoint.UValue = 0.0f;
						NewPoint.LaneDirection = 1;
						NewPoint.TurningRule = JunctionPoints[i].RightLanes[p].TurningRule;
						NewPoint.RoadType = JunctionPoints[i].RightLanes[p].RoadType;
						//DrawDebugPoint(GetWorld(), LaneBoundaryPositions[PointID], 20.0f, JunctionColorCodes[p], true, -1.0f, 5);

						RightHandPoints.Add(NewPoint);

					}

					//Sort our Points
					JunctionCapPoints.Append(RightHandPoints);
					CurrentJunctionPoints.Append(RightHandPoints);

					CurrentJunctionPoints.Sort([](const FCapPoints& A, const FCapPoints& B)
						{
							return A.PointType < B.PointType;
						});

					FJunctionIDPoints NewJunctionSortedPoints;
					NewJunctionSortedPoints.Points = CurrentJunctionPoints;

					JunctionIDSortedPoints.Add(NewJunctionSortedPoints);


					////TURNING LANE CENTER POINTS
					//FVector CenterLocationPoint = JunctionCenter + this->GetActorLocation();
					//TurningLaneCenterPoints = CurrentJunctionPoints;

					//for (int p = 0; p < TurningLaneCenterPoints.Num(); p++)
					//{
					//	FVector PointLocation = TurningLaneCenterPoints[p].Location - CenterLocationPoint;
					//	float Angle = FMath::Atan2(PointLocation.Y, PointLocation.X);
					//	TurningLaneCenterPoints[p].AngleFromCenter = Angle;
					//}

					////Sort Points by Angle Temp
					//TurningLaneCenterPoints.Sort([](const FCapPoints& A, const FCapPoints& B)
					//	{
					//		return A.AngleFromCenter < B.AngleFromCenter;
					//	});

					////Turning Lane Points
					//float tempVal = (1.0 / TurningLaneCenterPoints.Num());

					//for (int g = 0; g < TurningLaneCenterPoints.Num() - 1; g++)
					//{

					//	FVector CenterOfLane = (TurningLaneCenterPoints[g].Location + TurningLaneCenterPoints[g + 1].Location) * 0.5f;

					//	CenterOfLane = TurningLaneCenterPoints[g].Location;

					//	FJunctionTurningLanePoint TurningLanePoint;

					//	TurningLanePoint.JunctionID = i;
					//	TurningLanePoint.Location = CenterOfLane;
					//	TurningLanePoint.LaneDirection = TurningLaneCenterPoints[g + 1].LaneDirection;
					//	TurningLanePoint.RoadType = TurningLaneCenterPoints[g].RoadType;
					//	TurningLanePoint.TurningRule = TurningLaneCenterPoints[g].TurningRule;

					//	TurningLanePoints.Add(TurningLanePoint);

					//	int ColorIndex = 0;

					//	if (TurningLaneCenterPoints[g].RoadType == ELaneDrivingType::DRIVING)
					//	{
					//		ColorIndex = 1;
					//	}

					//	if (TurningLaneCenterPoints[g].RoadType == ELaneDrivingType::SHOULDER)
					//	{
					//		ColorIndex = 2;
					//	}


					//	//DrawDebugPoint(GetWorld(), CenterOfLane, 10.0f, JunctionColorCodes[ColorIndex], true, -1.0f, 2);


					//}

				}

			}


		}

		//CREATE INTERIOR JUNCTION POINTS

		ManualEditBuildCenterGeo();
		//if (CreateCenterLaneMarkings == true)
		//{
		//	ManualEditBuildGenterMarkings();
		//}
		////Create Traffic Lane Data
		//
		//if (CreateTurningLanes == true)
		//{
		//	CreateTurningLanePoints();
		//}

	}

}

//Generate an Axis Aligned Bounds
FVector AJunctionSurface::GenerateJunctionBounds()
{
	return FVector(0, 0, 0);
}

void AJunctionSurface::DrawVertices(TArray<FVector>Vertices)
{
	for (int i = 0; i < Vertices.Num(); i++)
	{
		DrawDebugPoint(GetWorld(), Vertices[i], 10.0f, FColor::Magenta, true, -1.0f, 3);
	}
}

void AJunctionSurface::ManualEditBuildCenterGeo()
{

	if (JunctionIDSortedPoints.IsEmpty())
	{
		return;
	}

	//Find the offset forward for where each junction lane intersects its neighbour road
	//Solve Junction index order (in case user doesnt make roads in correct order
	//Solve T junction where corner point must be lerp between two points

	//First Get Order of our Incoming Junction Roads
	TArray<FJunctionOrder> JunctionOrder;

	//Solve Angle From Center for all our cap points
	ManualEditPointsAngleFromCenter();

	//Go Through and just Isolate Edge Points
	TArray<FCapPoints> JunctionCapEdgePoints;
	for (int i = 0; i < JunctionCapPoints.Num(); i++)
	{

		if (JunctionCapPoints[i].PointType == -1 || JunctionCapPoints[i].PointType == 1)
		{
			JunctionCapEdgePoints.Add(JunctionCapPoints[i]);
			//DrawDebugPoint(GetWorld(), JunctionCapPoints[i].Location, 20.0f, FColor::Red, true, -1.0, 5);
			//UE_LOG(LogTemp, Warning, TEXT("Drawing Point, %i"),i);

		}

	}

	//Now Find where Each of our Isolated Edge Points Intersects with its neighbour point
	//We should start at the first point, check if point is right side, then see where it intersects with the next point in the order
	FoundIntersectPoints.Empty();
	IntersectCornerPoints.Empty();
	BezierEdgePoints.Empty();

	for (int i = 0; i < JunctionCapEdgePoints.Num(); i++)
	{
		//Check if Point is Left or Right
		if (JunctionCapEdgePoints[i].PointType == 1)
		{
			continue;
		}

		int NextPointIndex = (i + 1) % JunctionCapEdgePoints.Num();

		FVector PointAStart = JunctionCapEdgePoints[i].Location;
		FVector PointAEnd = JunctionCapEdgePoints[i].Location + (JunctionCapEdgePoints[i].ForwardVector * -10000.0f);
		FVector PointBStart = JunctionCapEdgePoints[NextPointIndex].Location;
		FVector PointBEnd = JunctionCapEdgePoints[NextPointIndex].Location + (JunctionCapEdgePoints[NextPointIndex].ForwardVector * -10000.0f);
		FVector IntersectPoint = LineIntersection(PointAStart, PointAEnd, PointBStart, PointBEnd);

		//Check Intersect Point 0 length means line intersct could not be found.
		if (IntersectPoint.Length() == 0)
		{
			//Find Halfway Point
			FVector HalfwayPoint = FMath::Lerp(PointAStart, PointBStart, 0.5f);
			IntersectPoint = HalfwayPoint;
		}

		float IntersectDistanceFirstPoint = FVector::Distance(PointAStart, IntersectPoint);
		float IntersectDistanceSecondPoint = FVector::Distance(PointBStart, IntersectPoint);

		//Apply Intersct Point IF found to a custom array
		FIntersectPoints P0;
		P0.Location = IntersectPoint;
		P0.IntersectedPointID = JunctionCapEdgePoints[i].PointID;
		P0.JunctionID = JunctionCapEdgePoints[i].JunctionID;
		P0.PointType = JunctionCapEdgePoints[i].PointType;
		P0.IntersectDistance = IntersectDistanceFirstPoint;
		P0.ForwardVector = JunctionCapEdgePoints[i].ForwardVector;
		
		FIntersectPoints P1;
		P1.Location = IntersectPoint;
		P1.IntersectedPointID = JunctionCapEdgePoints[NextPointIndex].PointID;
		P1.JunctionID = JunctionCapEdgePoints[NextPointIndex].JunctionID;
		P1.PointType = JunctionCapEdgePoints[NextPointIndex].PointType;
		P1.IntersectDistance = IntersectDistanceSecondPoint;
		P1.ForwardVector = JunctionCapEdgePoints[NextPointIndex].ForwardVector;

		FoundIntersectPoints.Add(P0);
		FoundIntersectPoints.Add(P1);

		//Apply The same three points to another array, so we can use this to create our edge geo later
		//Apply in a specific order which makes 
		FJunctionCapCornerPoints CornerPoints;

		TArray<FVector> CornerPointLocations;
		TArray<int> CornerPointIDs;
		TArray<int> SourceJunctionIDs;

		CornerPointLocations.Add(IntersectPoint);
		CornerPointLocations.Add(PointAStart);
		CornerPointLocations.Add(PointBStart);

		CornerPointIDs.Add(0);
		CornerPointIDs.Add(JunctionCapEdgePoints[i].PointID);
		CornerPointIDs.Add(JunctionCapEdgePoints[NextPointIndex].PointID);

		//This data needs to be passed all the way down to our bezier corner points
		SourceJunctionIDs.Add(-1);
		SourceJunctionIDs.Add(JunctionCapEdgePoints[i].JunctionID);
		SourceJunctionIDs.Add(JunctionCapEdgePoints[NextPointIndex].JunctionID);

		CornerPoints.Location.Append(CornerPointLocations);
		CornerPoints.PointID.Append(CornerPointIDs);
		CornerPoints.JunctionIDs.Append(SourceJunctionIDs);

		IntersectCornerPoints.Add(CornerPoints);

	}

	//Sort our Intersect Points
	FoundIntersectPoints.Sort([](const FIntersectPoints& PointA, const FIntersectPoints& PointB)
	{
			if (PointA.JunctionID != PointB.JunctionID)
			{
				return PointA.JunctionID < PointB.JunctionID;
			}

			return PointA.PointType < PointB.PointType;
	});


	//For each junction point. Get its points and offset them along the forward vector by the distance to the intersect points
	//Distance for each corner can be different. So for each middle point we need to find offset by lerping between our two offsets.
	//
	// NOTES: for all our endcap points. Try sort our points by angle and then ID? 

	//Apply U Value to all Junction Points
	//Also apply offset value based on lerp(leftPDistance, rightPDistance, UValue)


	for (int i = 0; i < JunctionIDSortedPoints.Num(); i++)
	{

		if (JunctionIDSortedPoints.IsEmpty())
		{
			return;
		}

		int SortedPointCount = JunctionIDSortedPoints[i].Points.Num();

		FVector StartLocation = JunctionIDSortedPoints[i].Points[0].Location;
		FVector EndLocation = JunctionIDSortedPoints[i].Points[SortedPointCount - 2].Location;
		float MaxDistance = FVector::Distance(StartLocation, EndLocation);

		//Find our First and Last Intersected Point
		FIntersectPoints IntersectPointA = FoundIntersectPoints[i * 2];
		FIntersectPoints IntersectPointB = FoundIntersectPoints[(i * 2) + 1];

		//Calculate New Forward Vectors to our Intersect Points
		FVector StartForwardVector = IntersectPointA.Location - StartLocation;
		StartForwardVector.Normalize();
		FVector EndForwardVector = IntersectPointB.Location - EndLocation;
		EndForwardVector.Normalize();

		//For Each Point, see where its distance falls between two points
		for (int j = 0; j < SortedPointCount; j++)
		{
			//Current Point Location
			FVector CurrentPointLocation = JunctionIDSortedPoints[i].Points[j].Location;

			float CurrentDistance = FVector::Distance(StartLocation, CurrentPointLocation);
			float NormalizedDistance = CurrentDistance / MaxDistance;

			JunctionIDSortedPoints[i].Points[j].UValue = NormalizedDistance;
			JunctionIDSortedPoints[i].Points[j].OffsetDistance = FMath::Lerp(IntersectPointA.IntersectDistance, IntersectPointB.IntersectDistance, NormalizedDistance);
			JunctionIDSortedPoints[i].Points[j].ForwardVector = FMath::Lerp(-StartForwardVector, -EndForwardVector, NormalizedDistance);

		}

	}

	TArray<FVector> InnerMostPoints;

	//For each Junction now, create our bit of geo 
	for (int i = 0; i < JunctionIDSortedPoints.Num(); i++)
	{

		int SortedPointCount = JunctionIDSortedPoints[i].Points.Num();
		TArray<FVector> JunctionSectionPoints;

		FJunctionIDPoints CurrentJunctionPoints;
		CurrentJunctionPoints = JunctionIDSortedPoints[i];

		CurrentJunctionPoints.Points.Sort([](const FCapPoints& PointA, const FCapPoints& PointB)
		{
				return PointA.UValue < PointB.UValue;
		});


		for (int j = 0; j < SortedPointCount; j++)
		{
			JunctionSectionPoints.Add(CurrentJunctionPoints.Points[j].Location);
		}

		for (int j = 0; j < SortedPointCount; j++)
		{
			const FVector SortedPointLocation = CurrentJunctionPoints.Points[j].Location;
			const float Distance = CurrentJunctionPoints.Points[j].OffsetDistance;
			const FVector Direction = CurrentJunctionPoints.Points[j].ForwardVector;

			FVector OffsetPosition = SortedPointLocation + (-Direction * Distance);

			JunctionSectionPoints.Add(OffsetPosition);

			//Add all our inner most points to their own list
			InnerMostPoints.Add(OffsetPosition);

		}

		//This is our vertex list
		//JunctionSectionPoints.Append(OffsetPoints);
		//Get Triangle List
		TArray<int> Triangles = ReturnTriangleIndicesGrid(JunctionSectionPoints, SortedPointCount, false);

		JunctionCenterSurface->CreateMeshSection(i, JunctionSectionPoints, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
		JunctionCenterSurface->SetMaterial(i,JunctionSurfaceMaterial);

	}

	InnerMostPoints.Add(JunctionCenter + this->GetActorLocation());
	
	//Create Center Cap Geo (Triangle Fan for now)
	int JunctionCenterIndex = JunctionCenterSurface->GetNumSections();
	TArray<int>JunctionCenterTriangles = ReturnTriangleIndicesFan(InnerMostPoints, InnerMostPoints.Num(), false);

	JunctionCenterSurface->CreateMeshSection(JunctionCenterIndex, InnerMostPoints, JunctionCenterTriangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	JunctionCenterSurface->SetMaterial(JunctionCenterIndex, JunctionSurfaceMaterial);

	//UE_LOG(LogTemp, Warning, TEXT("Intersect Corner Point Count: %i "), IntersectCornerPoints.Num());

	//CREATE OUR CORNER Flange Geo Bits To make Junction look better
	for (int i = 0; i < IntersectCornerPoints.Num(); i++)
	{

		//Make Bezier Curve
		TArray<FVector> BezierPoints;
		TArray<FVector> BezierNormal;
		
		BezierPoints.Add(IntersectCornerPoints[i].Location[0]);
		
		float IncrementT = 1.0 / 30;

		for (int j = 0; j < 31; j++)
		{
			FVector BezierCurveLocation = MathHelperFunctions::BezierCurvePosition(IntersectCornerPoints[i].Location[2], IntersectCornerPoints[i].Location[0], IntersectCornerPoints[i].Location[1], j * IncrementT);
			BezierPoints.Add(BezierCurveLocation);
			//DrawDebugPoint(GetWorld(), BezierCurveLocation, 20.0f, FColor::White, true, -1.0, 4);
		}

		//Put these points into their own array so we can reuse them for the edge lane markings and save recomputing all this junk
		FBezierCornerPoints CurrentBezierPoints;
		CurrentBezierPoints.CornerID = i;
		CurrentBezierPoints.Position.Append(BezierPoints);
		CurrentBezierPoints.StartJunctionID = IntersectCornerPoints[i].JunctionIDs[2];
		CurrentBezierPoints.EndJunctionID = IntersectCornerPoints[i].JunctionIDs[1];


		BezierEdgePoints.Add(CurrentBezierPoints);

		//Our Points are all in a specific order set by a previous function
		// 0 = Start 1 = Corner 2 = Middle
		TArray<int> Triangles = ReturnTriangleIndicesFan(BezierPoints, 0, false);
		
		////ADD DEBUG TOGGLE HERE
		//for (int j = 0; j < BezierPoints.Num(); j++)
		//{
		//	DrawDebugPoint(GetWorld(), BezierPoints[j], 20.0f, JunctionColorCodes[i], true, -1.0, i);

		//}

		JunctionCenterSurface->CreateMeshSection(JunctionCenterIndex + (1 + i ), BezierPoints, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
		JunctionCenterSurface->SetMaterial(JunctionCenterIndex + (1 + i), JunctionSurfaceMaterial);

	}

}

//Build the inner most lane markings 
void AJunctionSurface::ManualEditBuildGenterMarkings()
{

	if (BezierEdgePoints.Num() != 0)
	{

		//Calculate Normals for each point on the curve. This needs to be cleaned up as its very messy - or move this to the bezier curve formulation
		for (int i = 0; i < BezierEdgePoints.Num(); i++)
		{
			TArray<FVector> Normals;

			for (int j = 0; j < BezierEdgePoints[i].Position.Num() - 1; j++)
			{

				FVector Normal = BezierEdgePoints[i].Position[j] - BezierEdgePoints[i].Position[j + 1];

				if (j == BezierEdgePoints[i].Position.Num() - 1)
				{
					Normal = FVector(1, 0, 0); // BezierEdgePoints[i].Position[j - 1] - BezierEdgePoints[i].Position[j];
				}

				Normals.Add(Normal);
			}

			BezierEdgePoints[i].Normal.Append(Normals);
			BezierEdgePoints[i].Normal.Add(Normals[Normals.Num() - 1]);

		}

		float IncrementValue = 1.0 / (BezierEdgePoints.Num());

		int MarkingIndex = 1; //LaneMarkingsSurface->GetNumSections();

		//Draw all our Points <--- MERGE THIS INTO LOOP ABOVE
		for (int i = 0; i < BezierEdgePoints.Num(); i++)
		{

			//Calculate offsets from inner lane
			int StartJunctionID = BezierEdgePoints[i].StartJunctionID;
			float StartOffset = JunctionPoints[StartJunctionID].RightLanes[JunctionPoints[StartJunctionID].RightLanes.Num() - 1].LaneWidth;

			int EndJunctionID = BezierEdgePoints[i].EndJunctionID;
			float EndOffset = JunctionPoints[EndJunctionID].LeftLanes[JunctionPoints[EndJunctionID].LeftLanes.Num() - 1].LaneWidth;

			int BezierPointTotalCount = BezierEdgePoints[i].Position.Num();
			float IncrementLerpValue = 1.0 / (BezierPointTotalCount - 2);

			TArray<FVector> BezierMarkingVertexPositions;
			TArray<FVector2D> UV;

			for (int j = 1; j < BezierPointTotalCount; j++)
			{

				FVector BaseLocation = BezierEdgePoints[i].Position[j];
				FVector RightVector = FVector::CrossProduct(BezierEdgePoints[i].Normal[j], FVector(0, 0, 1));
				RightVector.Normalize();

				float OffsetDistance = FMath::Lerp(-StartOffset, EndOffset, IncrementLerpValue * j);

				FVector P0OffsetLocation = BaseLocation + (RightVector * (OffsetDistance - 15.0f));
				FVector P1OffsetLocation = BaseLocation + (RightVector * (OffsetDistance + 15.0f));

				BezierMarkingVertexPositions.Add(P0OffsetLocation + FVector(0, 0, 10.0f));
				BezierMarkingVertexPositions.Add(P1OffsetLocation + FVector(0, 0, 10.0f));

				//UV
				FVector2D P0UV = FVector2D(j, 0.0f);
				FVector2D P1UV = FVector2D(j, 0.5f);

				UV.Add(P0UV);
				UV.Add(P1UV);

				////DEBUG
				//float ColVal = (IncrementLerpValue * j) * 255;
				//DrawDebugPoint(GetWorld(), P0OffsetLocation, 5.0f, FColor(ColVal, ColVal, ColVal), true, -1.0, 5);
				//DrawDebugPoint(GetWorld(), P1OffsetLocation, 5.0f, FColor(ColVal, ColVal, ColVal), true, -1.0, 5);

			}

			TArray<int>TempIndices = ReturnTriangleIndicesGrid(BezierMarkingVertexPositions, 2, false);

			//LaneMarkingsSurface->CreateMeshSection(MarkingIndex + i, BezierMarkingVertexPositions, TempIndices, TArray<FVector>(), UV, TArray<FColor>(), TArray<FProcMeshTangent>(), false);

			if (GenericLaneMarkingMaterial != nullptr)
			{
				//LaneMarkingsSurface->SetMaterial(MarkingIndex + i, JunctionPoints[0].LeftLanes[0].LaneMarkings[0].MarkingMaterial);
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("No Material set for Lane Edge Markings"));
			}
			//LaneMarkingsSurface->bCastDynamicShadow = false;


		}

	}

}

//Use this to create turning lane point data
void AJunctionSurface::CreateTurningLanePoints()
{

	UE_LOG(LogTemp, Log, TEXT("Rebuilding Turning Lanes"));

	TArray<FJunctionTurningLanePoint>CreatedTurningLanePoints;
	TArray<FJunctionTurningLanePoint> StartPoints;
	TArray<FJunctionTurningLanePoint> EndPoints;
	TurningLaneConnections.Empty();

	if (JunctionCenterLineEndPoint.Num() == 0)
	{
		return;
	}
	
	//For Each Junction and its center point, create the turning lane marker for each lane 
	for(int i = 0; i < JunctionCenterLineEndPoint.Num(); i++)
	{
	
		DrawDebugPoint(GetWorld(), JunctionCenterLineEndPoint[i].Location, 10.0f, JunctionColorCodes[i], true, -1.0f, 2);

		//Get Lanes
		int CurrentSampledJunctionID = JunctionCenterLineEndPoint[i].JunctionID;
		int CurrentSampledLeftLaneCount = JunctionPoints[i].LeftLanes.Num();
		int CurrentSampledRightLaneCount = JunctionPoints[i].RightLanes.Num();
		
		//Current Lane Location
		FVector CurrentJunctionCenterLocation = JunctionCenterLineEndPoint[i].Location;
		FVector CurrentJunctionCenterRight = JunctionCenterLineEndPoint[i].RightVector;
		FVector CurrentJunctionForward = JunctionCenterLineEndPoint[i].ForwardVector;

		float AccumilatedRoadWidth = 0.0f;

		//For each Left Lane - make a point for its turning lane
		for (int j = 0; j < CurrentSampledLeftLaneCount; j++)
		{
			FJunctionLaneData CurrentLane = JunctionPoints[i].LeftLanes[j];
			float CurrentLaneWidth = CurrentLane.LaneWidth;
			
			//Get The Half Width of the Lane + Accumilated Lane Width so we get the center of each lane
			float LaneHalfWidth = CurrentLaneWidth * 0.5;
			FVector CurrentLaneCenterLocation = CurrentJunctionCenterLocation + (CurrentJunctionCenterRight * (AccumilatedRoadWidth + (CurrentLaneWidth * 0.5f)));

			//Create our New Point Data
			FJunctionTurningLanePoint NewTurningLanePoint;
			NewTurningLanePoint.JunctionID = CurrentSampledJunctionID;
			NewTurningLanePoint.LaneDirection = 0;
			NewTurningLanePoint.LaneID = j;
			NewTurningLanePoint.RoadType = CurrentLane.RoadType;
			NewTurningLanePoint.TurningRule = CurrentLane.TurningRule;
			NewTurningLanePoint.Location = CurrentLaneCenterLocation;
			NewTurningLanePoint.ForwardVector = CurrentJunctionForward;

			//If the Lane Type is Driving, add it to our list
			if (CurrentLane.RoadType == ELaneDrivingType::DRIVING)
			{
				CreatedTurningLanePoints.Add(NewTurningLanePoint);
				StartPoints.Add(NewTurningLanePoint);
			}

			//Increment our RoadWidth
			AccumilatedRoadWidth += CurrentLaneWidth;

		}

		AccumilatedRoadWidth = 0.0f;

		//Do the same for the Right Lanes also
		for (int j = 0; j < CurrentSampledRightLaneCount; j++)
		{
			FJunctionLaneData CurrentLane = JunctionPoints[i].RightLanes[j];
			float CurrentLaneWidth = CurrentLane.LaneWidth;

			//Get The Half Width of the Lane + Accumilated Lane Width so we get the center of each lane
			float LaneHalfWidth = CurrentLaneWidth * 0.5;
			FVector CurrentLaneCenterLocation = CurrentJunctionCenterLocation - (-CurrentJunctionCenterRight * (AccumilatedRoadWidth + (CurrentLaneWidth * 0.5f)));

			//Create our New Point Data
			FJunctionTurningLanePoint NewTurningLanePoint;
			NewTurningLanePoint.JunctionID = CurrentSampledJunctionID;
			NewTurningLanePoint.LaneDirection = 0;
			NewTurningLanePoint.LaneID = j;
			NewTurningLanePoint.RoadType = CurrentLane.RoadType;
			NewTurningLanePoint.TurningRule = CurrentLane.TurningRule;
			NewTurningLanePoint.Location = CurrentLaneCenterLocation;
			NewTurningLanePoint.ForwardVector = CurrentJunctionForward;

			//If the Lane Type is Driving, add it to our liset
			if (CurrentLane.RoadType == ELaneDrivingType::DRIVING)
			{
				CreatedTurningLanePoints.Add(NewTurningLanePoint);
				EndPoints.Add(NewTurningLanePoint);
			}

			//Increment our RoadWidth
			AccumilatedRoadWidth += CurrentLaneWidth;
		}

	}
	
	//Can Remove this
	for (int i = 0; i < CreatedTurningLanePoints.Num(); i++)
	{
		FVector CenterLoc = CreatedTurningLanePoints[i].Location;
		//UE_LOG(LogTemp, Warning, TEXT("Drawing Point %d"), i);
		DrawDebugPoint(GetWorld(), CenterLoc, 10.0f, FColor::Blue, true, 1.0f, 2);
	}


	for (int i = 0; i < StartPoints.Num(); i++)
	{
		FTurningLaneConnections NewConnection;
		TArray<FJunctionTurningLanePoint>FoundEndPoints;

		int CurrentJunctionID = StartPoints[i].JunctionID;

		FVector StartForwardVector = StartPoints[i].ForwardVector;
		//Rotate our Vector to the Right - so that we can check angle over 180 degrees
		StartForwardVector = FVector::CrossProduct(StartForwardVector, FVector(0, 0, 1));

		//Logic for Lane Detection
		ELaneTurningOptions CurrentLaneTurning = StartPoints[i].TurningRule;

		//DO LOGIC IN HERE FOR CHECKING CONNECTIONS
		for (int j = 0; j < EndPoints.Num(); j++)
		{

			if (EndPoints[j].JunctionID == CurrentJunctionID)
			{
				continue;
			}

			FVector EndForwardVector = EndPoints[j].ForwardVector;

			//Should give us a value between 0 and 2pi where left of the junction is 0 and right is pi
			float AngleBetweenPoints = FMath::Atan2(EndForwardVector.Y, EndForwardVector.X) - atan2(StartForwardVector.Y, StartForwardVector.X);
			AngleBetweenPoints = FMath::Abs(AngleBetweenPoints);

			//UE_LOG(LogTemp, Warning, TEXT("Debug Lane Connections: Start Point: %i, End Point %i, Angle: %f"), i, j, AngleBetweenPoints);

			//Do all our logic here for solving our lane types - expose the parameters/tolerances to a variable later
			switch (CurrentLaneTurning)
			{

			case ELaneTurningOptions::ALL:
				FoundEndPoints.Add(EndPoints[j]);
				break;
			case ELaneTurningOptions::LEFT:

				if (AngleBetweenPoints > 2.5f)
				{
					FoundEndPoints.Add(EndPoints[j]);
				}

				break;
			case ELaneTurningOptions::LEFTFORWARD:
				if (AngleBetweenPoints > 1.0f)
				{
					FoundEndPoints.Add(EndPoints[j]);
				}

				break;
			case ELaneTurningOptions::FORWARD:
				if (AngleBetweenPoints > 1.0f && AngleBetweenPoints < 2.5f)
				{
					FoundEndPoints.Add(EndPoints[j]);
				}


				break;
			case ELaneTurningOptions::FORWARDRIGHT:
				if (AngleBetweenPoints < 2.0f)
				{
					FoundEndPoints.Add(EndPoints[j]);
				}

				break;
			case ELaneTurningOptions::RIGHT:
				if (AngleBetweenPoints < 1.0f)
				{
					FoundEndPoints.Add(EndPoints[j]);
				}

				break;
			default:
				FoundEndPoints.Add(EndPoints[j]);
				break;
			}
		}

		if (FoundEndPoints.Num() > 0)
		{
			NewConnection.StartPoint = StartPoints[i];
			NewConnection.EndPoints.Append(FoundEndPoints);
			TurningLaneConnections.Add(NewConnection);
		}
	}


	//Debug Connections
	if (DrawLaneConnections == true && TurningLaneConnections.Num() != 0)
	{
		if (DebugLaneConnectionID < TurningLaneConnections.Num())
		{
			FVector StartLineLocation = TurningLaneConnections[DebugLaneConnectionID].StartPoint.Location;
			for (int i = 0; i < TurningLaneConnections[DebugLaneConnectionID].EndPoints.Num(); i++)
			{
				FVector EndLocation = TurningLaneConnections[DebugLaneConnectionID].EndPoints[i].Location;
				DrawDebugLine(GetWorld(), StartLineLocation, EndLocation,JunctionColorCodes[i], true, -1.0f, 2, 10.0f);
			}
		}
	}

	//For each of our connections we need to now make a spline and piece of geometry
	//Spline for our traffic sim
	//Geo for our rendering

	//Force Empty for now, but we might want to keep a reference to existing data and only update what we need to
	TurningLanes.Empty();

	for (int i = 0; i < TurningLaneConnections.Num(); i++)
	{

		//For each Connection, make a new lane from its start to end points
		//find its intersection/middle point
		//create a bezier curve
		//add it to our turning lane array

		
		for (int j = 0; j < TurningLaneConnections[i].EndPoints.Num(); j++)
		{
			//Find intersect point
			
			FVector PointAEnd = TurningLaneConnections[i].StartPoint.Location + (-TurningLaneConnections[i].StartPoint.ForwardVector * 20000.0f);
			FVector PointBEnd = TurningLaneConnections[i].EndPoints[j].Location + (-TurningLaneConnections[i].EndPoints[j].ForwardVector * 20000.0f);
			
			//Try to find intersect point

			FVector IntersectPoint = LineIntersection(TurningLaneConnections[i].StartPoint.Location, PointAEnd, TurningLaneConnections[i].EndPoints[j].Location, PointBEnd);

			DrawDebugPoint(GetWorld(), TurningLaneConnections[i].StartPoint.Location, 10.0f, FColor::Blue, true, -1.0f, 2);
			DrawDebugPoint(GetWorld(), TurningLaneConnections[i].EndPoints[j].Location, 10.0f, FColor::Red, true, -1.0f, 2);			
			DrawDebugPoint(GetWorld(), PointAEnd, 10.0f, FColor::Orange, true, -1.0f, 2);
			DrawDebugPoint(GetWorld(), PointBEnd, 10.0f, FColor::Green, true, -1.0f, 2);


			if (IntersectPoint.Length() == 0.0)
			{
				IntersectPoint = FMath::Lerp(TurningLaneConnections[i].StartPoint.Location, TurningLaneConnections[i].EndPoints[j].Location, 0.5f);
			}

			//DrawDebugPoint(GetWorld(), IntersectPoint, 10.0f, FColor::White, true, -1.0f, 2);

			TArray<FVector> BezierPoints;
			TArray<FVector> BezierNormals;

			//Create Bezier Curve Points
			float IncrementT = 1.0 / 30;

			for (int k = 0; k < 31; k++)
			{
				//Original
				//FVector BezierCurveLocation = MathHelperFunctions::BezierCurvePosition(TurningLaneConnections[i].EndPoints[j].Location, IntersectPoint, TurningLaneConnections[i].StartPoint.Location, k * IncrementT);
				FVector BezierCurveLocation = MathHelperFunctions::BezierCurvePosition(TurningLaneConnections[i].StartPoint.Location, IntersectPoint, TurningLaneConnections[i].EndPoints[j].Location, k * IncrementT);

				//Original
				//FVector SecondBezierCurvelocation = MathHelperFunctions::BezierCurvePosition(TurningLaneConnections[i].EndPoints[j].Location, IntersectPoint, TurningLaneConnections[i].StartPoint.Location, (k + 1) * IncrementT);
				FVector SecondBezierCurvelocation = MathHelperFunctions::BezierCurvePosition(TurningLaneConnections[i].StartPoint.Location, IntersectPoint, TurningLaneConnections[i].EndPoints[j].Location, (k + 1) * IncrementT);

				FVector TempNormal = BezierCurveLocation - SecondBezierCurvelocation;
				TempNormal.Normalize();

				BezierPoints.Add(BezierCurveLocation);
				BezierNormals.Add(TempNormal);
				//DrawDebugPoint(GetWorld(), BezierCurveLocation, 20.0f, JunctionColorCodes[TurningLaneConnections[i].StartPoint.JunctionID], true, -1.0, 4);
			}

			FJunctionTurningLane NewTurningLane;

			NewTurningLane.TurningLaneID = j;
			NewTurningLane.TurningLanePoints = BezierPoints;
			NewTurningLane.TurningLanePointNormal = BezierNormals;

			TurningLanes.Add(NewTurningLane);
		}

	}

	


	////Make Geo
	//for (int i = 0; i < TurningLanes.Num(); i++)
	//{

	//	//Create Vertices
	//	TArray<FVector> TurningLaneVerts;
	//	TArray<int> TurningLaneIndices;
	//	TArray<FVector2D> UV;
	//	
	//	int PointCount = TurningLanes[i].TurningLanePoints.Num();

	//	for (int j = 0; j < PointCount; j++)
	//	{
	//		//Create Left and Right Points
	//		FVector RightVector = FVector::CrossProduct(TurningLanes[i].TurningLanePointNormal[j],FVector(0,0,1));

	//		FVector P0 = TurningLanes[i].TurningLanePoints[j] + (RightVector * 175.0f); //175 is half our usual lane width - fix this
	//		FVector P1 = TurningLanes[i].TurningLanePoints[j] + (-RightVector * 175.0f);

	//		FVector VerticalLift = FVector(0.0f, 0.0f, 0.1f);

	//		TurningLaneVerts.Add(P0 + VerticalLift);
	//		TurningLaneVerts.Add(P1 + VerticalLift);

	//		//UV
	//		FVector2D P0UV = FVector2D(j, 0.0f);
	//		FVector2D P1UV = FVector2D(j, 1.0f);

	//		UV.Add(P0UV);
	//		UV.Add(P1UV);


	//	}
	//	
	//	//Generate Indices
	//	TurningLaneIndices = ReturnTriangleIndicesGrid(TurningLaneVerts, 2, true);

	//	//TurningLanesSurface->CreateMeshSection(i, TurningLaneVerts, TurningLaneIndices, TArray<FVector>(), UV, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	//	//TurningLanesSurface->SetMaterial(i, TurningLaneSurfaceMaterial);
	//	//TurningLanesSurface->CastShadow = false;

	//}

}

FVector AJunctionSurface::LineIntersection(FVector PointAStart, FVector PointAEnd, FVector PointBStart, FVector PointBEnd)
{

	FVector s1 = PointAEnd - PointAStart;
	FVector s2 = PointBEnd - PointBStart;

	float s;
	float t;

	s = (-s1.Y * (PointAStart.X - PointBStart.X) + s1.X * (PointAStart.Y - PointBStart.Y)) / (-s2.X * s1.Y + s1.X * s2.Y);
	t = (s2.X * (PointAStart.Y - PointBStart.Y) - s2.Y * (PointAStart.X - PointBStart.X)) / (-s2.X * s1.Y + s1.X * s2.Y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{

		return PointAStart + (t * s1);
	}

	return FVector::ZeroVector; //No Point

}

//Run this to get the angle of each point from the center
void AJunctionSurface::ManualEditPointsAngleFromCenter()
{

	//For Each Point go through it and get its angle from the center of the junction point
	FVector CenterLocationPoint = JunctionCenter + this->GetActorLocation();

	//DrawDebugPoint(GetWorld(), CenterLocationPoint, 50.0f, FColor::Red, true, -1.0, 5);


	for (int i = 0; i < JunctionCapPoints.Num(); i++)
	{
		FVector PointLocation = JunctionCapPoints[i].Location - CenterLocationPoint;
		float Angle = FMath::Atan2(PointLocation.Y, PointLocation.X);
		JunctionCapPoints[i].AngleFromCenter = Angle;
	}


	//Sort Points by Angle Temp
	JunctionCapPoints.Sort([](const FCapPoints& A, const FCapPoints& B)
	{
			return A.AngleFromCenter < B.AngleFromCenter;
	});


	float IncrementValue = 1.0 / JunctionCapPoints.Num();


	//for(int i = 0; i < JunctionCapPoints.Num(); i++)
	//{ 
	//	FColor PointColour = FColor((IncrementValue * i) * 255, (IncrementValue * i) * 255, (IncrementValue * i) * 255);
	//	DrawDebugPoint(GetWorld(), JunctionCapPoints[i].Location, 20.0f, PointColour, true, -1.0, 5);
	//}

	//Debug Draw Points in the Order for Viewing
}

//Find where our points 
void AJunctionSurface::ManualEditFindIntersectPoints()
{



}


//When Editing Manually Reset some Properties back to our struct so we can sample them for construction
void AJunctionSurface::ManualEditInitialiseJunction()
{
	if (JunctionPoints.Num() == 0)
	{
		return;
	}

	//Get Junction WorldSpace Position
	JunctionWorldLocation = this->GetActorLocation();


	for (int i = 0; i < JunctionPoints.Num(); i++)
	{
		FVector ForwardVector = (JunctionPoints[i].Location + JunctionWorldLocation) - (JunctionCenter + JunctionWorldLocation);
		ForwardVector.Normalize(0.2);
		FVector RightVector = FVector::CrossProduct(ForwardVector, FVector(0, 0, 1));

		JunctionPoints[i].ForwardVector = ForwardVector;
		JunctionPoints[i].RightVector = RightVector;
		JunctionPoints[i].EndLocation = FMath::Lerp(JunctionPoints[i].Location + JunctionWorldLocation, JunctionCenter + JunctionWorldLocation, JunctionPoints[i].ULength);

	}

	//Create Center Points
	for (int i = 0; i < JunctionPoints.Num(); i++)
	{
		JunctionPoints[i].CenterLinePoints = ManualEditCreateCenterLine(JunctionPoints[i]);
	}

}

//Create our Center Line Points
TArray<FLaneCenterLinePoints> AJunctionSurface::ManualEditCreateCenterLine(FJunctionPoint Junction)
{

		TArray<FLaneCenterLinePoints> CenterLinePoints;

		float JunctionLength = FVector::Distance(Junction.Location + JunctionWorldLocation, Junction.EndLocation);
		float DistanceBetweenPoints = 1.0 / Junction.UResolution;
		
		for (int i = 0; i < Junction.UResolution; i++)
		{
			FLaneCenterLinePoints Point;

			FVector PointPosition = FMath::Lerp(Junction.Location + JunctionWorldLocation, Junction.EndLocation, i * DistanceBetweenPoints);
			FVector PointNormal = (Junction.Location + JunctionWorldLocation) - Junction.EndLocation;
			PointNormal.Normalize(0.02);
			FVector PointRightVector = FVector::CrossProduct(PointNormal, FVector(0, 0, 1));

			Point.Location = PointPosition;
			Point.ForwardVector = PointNormal;
			Point.RightVector = PointRightVector;
			
			CenterLinePoints.Add(Point);
			//DrawDebugPoint(GetWorld(), PointPosition, 10.0f, FColor::White, true, -1.0f, 2);
		}

		return CenterLinePoints;

}

TArray<int> AJunctionSurface::ReturnTriangleIndicesFan(TArray<FVector> Vertices, int RootPointIndex, bool ReverseWindingOrder)
{
	TArray<int> Indices;

	//We need to go through all our vertex positions and generate triangles - its a bit anoying we need to use a formula to always generate clean quads
	//Right now - we generate one quad per road point - so each roadpoint requires SIX indices = roadPoints.Num * 6

	int pointCount = Vertices.Num(); //SplinePositions.Num();  // RoadPoints.Num();
	//int widthPoints = ColumnCount; //Set this to properly use how many spans we make
	int rootIndex = 0;

	if (ReverseWindingOrder == false)
	{

		//Each Step Along Road 
		for (int i = 0; i < pointCount; i++)
		{

			//if (i == RootPointIndex)
			//{
			//	continue;
			//}

			int first = RootPointIndex;
			int second = i;
			int third = (i + 1) % (pointCount);

			Indices.Add(first);
			Indices.Add(second);
			Indices.Add(third);

		}
	}

	return Indices;
}

TArray<int> AJunctionSurface::ReturnTriangleIndicesGrid(TArray<FVector> Vertices, int ColumnCount, bool ReverseWindingOrder)
{
	TArray<int> Indices;

	//We need to go through all our vertex positions and generate triangles - its a bit anoying we need to use a formula to always generate clean quads
	//Right now - we generate one quad per road point - so each roadpoint requires SIX indices = roadPoints.Num * 6

	int pointCount = Vertices.Num(); //SplinePositions.Num();  // RoadPoints.Num();
	int widthPoints = ColumnCount; //Set this to properly use how many spans we make
	int rootIndex = 0;

	if (ReverseWindingOrder == false)
	{

		//Each Step Along Road (CHANGE i here to however many sub increments we need to make)
		for (int i = 0; i < pointCount; i++)
		{
			for (int j = 0; j < (widthPoints - 1); j++)
			{
				//FIRST TRIANGLE
				int first = (rootIndex + j);
				int second = (rootIndex + j) + 1;
				int third = (rootIndex + j) + widthPoints;

				Indices.Add(first);
				Indices.Add(second);
				Indices.Add(third);

				//SECOND TRIANGLE
				int fourth = (rootIndex + j) + 1;
				int fifth = (rootIndex + j) + (widthPoints + 1);
				int sixth = (rootIndex + j) + widthPoints;

				Indices.Add(fourth);
				Indices.Add(fifth);
				Indices.Add(sixth);
			}
			rootIndex = i * widthPoints;
		}
	}
	else
	{
		//Each Step Along Road 
		for (int i = 0; i < pointCount; i++)
		{

			for (int j = 0; j < (widthPoints - 1); j++)
			{
				//FIRST TRIANGLE
				int first = (rootIndex + j);
				int second = (rootIndex + j) + 1;
				int third = (rootIndex + j) + widthPoints;



				//SECOND TRIANGLE
				int fourth = (rootIndex + j) + 1;
				int fifth = (rootIndex + j) + (widthPoints + 1);
				int sixth = (rootIndex + j) + widthPoints;

				Indices.Add(second);
				Indices.Add(first);
				Indices.Add(third);


				Indices.Add(fifth);
				Indices.Add(fourth);
				Indices.Add(sixth);
			}
			rootIndex = i * widthPoints;
		}
	}

	return Indices;
}

//DEP THIS WITH PROPER FUNCTION ABOVE
void AJunctionSurface::ManualEditCreateLaneTriangles(int PointCount, bool ReverseWinding)
{
	TriangleIndices.Empty();

	//We need to go through all our vertex positions and generate triangles - its a bit anoying we need to use a formula to always generate clean quads
	//Right now - we generate one quad per road point - so each roadpoint requires SIX indices = roadPoints.Num * 6

	int vertexCount = VertexPositions.Num();
	int pointCount = PointCount; //SplinePositions.Num();  // RoadPoints.Num();

	int widthPoints = 2; //Set this to properly use how many spans we make

	int rootIndex = 0;

	if (ReverseWinding == false)
	{

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

	}
	else
	{

		//Each Step Along Road 
		for (int i = 0; i < pointCount; i++)
		{

			for (int j = 0; j < (widthPoints - 1); j++)
			{
				//FIRST TRIANGLE
				int first = (rootIndex + j);
				int second = (rootIndex + j) + 1;
				int third = (rootIndex + j) + widthPoints;

				//SECOND TRIANGLE
				int fourth = (rootIndex + j) + 1;
				int fifth = (rootIndex + j) + (widthPoints + 1);
				int sixth = (rootIndex + j) + widthPoints;

				TriangleIndices.Add(second);
				TriangleIndices.Add(first);
				TriangleIndices.Add(third);


				TriangleIndices.Add(fifth);
				TriangleIndices.Add(fourth);
				TriangleIndices.Add(sixth);
			}
			rootIndex = i * widthPoints;
		}
	}
	return;
}

void AJunctionSurface::ManualEditCreateLaneBoundaries(TArray<FJunctionLaneData> Lanes, TArray<FLaneCenterLinePoints> CenterLinePoints)
{

	//VertexPositions.Empty();
	LaneBoundaryPositions.Empty();

	TArray<FVector> LaneCrossSectionPositions;

	//Create a set of Verts and UVs for This Lane

	int PointCount = CenterLinePoints.Num();
	float Uincrement = 1.0 / PointCount;

	//For Each Point along our Spine
	for (int i = 0; i < PointCount; i++)
	{
		LaneCrossSectionPositions.Empty();
		LaneCrossSectionPositions.Add(CenterLinePoints[i].Location);

		float UValue = Uincrement * i;
		
		float AccumiliatedWidth = 0.0;
		//For Each Lane - Get the current accumulated width, and create a point <- Do this all in one go, to save us some speed later
		for (int j = 0; j < Lanes.Num(); j++)
		{

			float StartWidth = Lanes[j].Start;
			float EndWidth = Lanes[j].End;

			//Get our Width based on our Quadratic Ease In/Out Function * TotalLaneWidth (This is our offset
			float Width = Lanes[j].LaneWidth * (FMath::Clamp(MathHelperFunctions::EaseInOutQuad(StartWidth, EndWidth, UValue) + (1.0 - Lanes[j].LaneFormingStrength), 0.0f, 1.0f));

			//Append our new Width Value so the nex point is always offset from the previous lane
			AccumiliatedWidth = AccumiliatedWidth + Width;

			//Create our Point Position for this Lane
			FVector PointLocation = CenterLinePoints[i].Location + (CenterLinePoints[i].RightVector * AccumiliatedWidth);

			LaneCrossSectionPositions.Add(PointLocation);
			//DrawDebugPoint(GetWorld(), PointLocation, 5.0f, FColor::Orange, true, -1.0f, 8);

		}

		//Reverse Point Sorting for Left Lanes

		LaneBoundaryPositions.Append(LaneCrossSectionPositions);

	}

}

void AJunctionSurface::ManualEditCreateLaneMarkingVertices(int index, int PointCount, int LaneCount, float LaneLength, FVector2D UVTiling, FVector2D UVOffset, float MarkingWidth, float MarkingOffset)
{
	LaneMarkingVertices.Empty();


	UVs.Empty();

	//Add more code here to add points in between boundaries to increase the lane geo res along the V 

	for (int i = 0; i < PointCount; i++)
	{
		int offset = i * (LaneCount + 1);

		//Lift the Vertices ---
		FVector VerticalOffset = FVector(0, 0, 0.1);

		FVector LaneSVector = LaneBoundaryPositions[offset + index] - LaneBoundaryPositions[offset + (index + 1)];
		LaneSVector.Normalize();

		FVector FirstPoint = LaneBoundaryPositions[offset + index] + (LaneSVector * MarkingOffset);

		//First Vert
		LaneMarkingVertices.Add(FirstPoint + VerticalOffset);
		
		//Offset Second Vertex
		FVector SecondPoint = FirstPoint + (LaneSVector * MarkingWidth);

		LaneMarkingVertices.Add(SecondPoint + VerticalOffset);

		float PositionAlongSpline = i;

		//FVector2D UV0 = FVector2D(PositionAlongSpline, 1.0f);
		//LaneMarkingUVs.Add((UV0 * Lanes[LaneIndex].LaneMarkings[0].UVTiling) + Lanes[LaneIndex].LaneMarkings[0].UVOffset);
		//FVector2D UV1 = FVector2D(PositionAlongSpline, 0.0f);
		//LaneMarkingUVs.Add((UV1 * Lanes[LaneIndex].LaneMarkings[0].UVTiling) + Lanes[LaneIndex].LaneMarkings[0].UVOffset);

		//UVs
		FVector2D UV0 = FVector2D(i, 1.0f);
		FVector2D UV1 = FVector2D(i, 0.0f);

		UVs.Add((UV0 * UVTiling) + UVOffset);
		UVs.Add((UV1 * UVTiling) + UVOffset);

	}


}

//Go through our Lane boundaries and create the proper vertices for the lane to make
void AJunctionSurface::ManualEditCreateLaneVertices(int index, int PointCount, int LaneCount, float LaneLength, FVector2D UVTiling, FVector2D UVOffset)
{


	VertexPositions.Empty();
	UVs.Empty();

	//Add more code here to add points in between boundaries to increase the lane geo res along the V 

	for (int i = 0; i < PointCount; i++)
	{
		int offset = i * (LaneCount + 1);

		VertexPositions.Add(LaneBoundaryPositions[offset + index] - JunctionWorldLocation);
		VertexPositions.Add(LaneBoundaryPositions[offset + (index + 1)] - JunctionWorldLocation);


		float PositionAlongSpline = i;

		//FVector2D UV0 = FVector2D(PositionAlongSpline, 1.0f);
		//LaneMarkingUVs.Add((UV0 * Lanes[LaneIndex].LaneMarkings[0].UVTiling) + Lanes[LaneIndex].LaneMarkings[0].UVOffset);
		//FVector2D UV1 = FVector2D(PositionAlongSpline, 0.0f);
		//LaneMarkingUVs.Add((UV1 * Lanes[LaneIndex].LaneMarkings[0].UVTiling) + Lanes[LaneIndex].LaneMarkings[0].UVOffset);

		//UVs
		FVector2D UV0 = FVector2D(i, 1.0f);
		FVector2D UV1 = FVector2D(i, 0.0f);
		
		UVs.Add((UV0 * UVTiling) + UVOffset);
		UVs.Add((UV1 * UVTiling) + UVOffset);


	}

}

//This is for manual editor mode - draw debug helpers to visualise junction
void AJunctionSurface::ManualEditDrawJunctionShape()
{

	//FVector JunctionWorldLocation = this->GetActorLocation();
	DebugPointPositions.Empty();

	if (JunctionPoints.Num() == 0)
	{
		return;
	}


	int UResolution = 30;

	float testlerp = MathHelperFunctions::SimpleLerpTest(1.0f, 2.0f);

	//Draw Center Lines Lines
	for (int i = 0; i < JunctionPoints.Num(); i++)
	{

		for (int r = 0; r < UResolution; r++)
		{

			//Lerp Between U Values
			float IncrementValue = 1.0 / UResolution;
			float UValue = FMath::Lerp(0.0, JunctionPoints[i].ULength, r * IncrementValue);

			//FVector RoadStartP = JunctionPoints[i].Location + JunctionWorldLocation;
			FVector RoadStartPosition = FMath::Lerp(JunctionPoints[i].Location + JunctionWorldLocation, JunctionCenter + JunctionWorldLocation, UValue);



			//Draw Center Line
			DrawDebugLine(GetWorld(), RoadStartPosition, JunctionCenter + JunctionWorldLocation, JunctionColorCodes[i], true, 100.0f, 1, 10.0f);

			float LeftAccumilatedWidth = 0.0f;

			for (int j = 0; j < JunctionPoints[i].LeftLanes.Num(); j++)
			{
				
				float StartWidth = JunctionPoints[i].LeftLanes[j].Start;
				float EndWidth = JunctionPoints[i].LeftLanes[j].End;

				float WidthScalar = FMath::Clamp(FMath::Lerp(StartWidth, EndWidth, r * IncrementValue),0.0f,1.0f);
				float QuadWidthScalar = FMath::Clamp(MathHelperFunctions::EaseInOutQuad(StartWidth, EndWidth,r * IncrementValue),0.0f,1.0f);


				LeftAccumilatedWidth = LeftAccumilatedWidth + (JunctionPoints[i].LeftLanes[j].LaneWidth * QuadWidthScalar);
				FVector DrawStartPointLocation = RoadStartPosition + (JunctionPoints[i].RightVector * LeftAccumilatedWidth);
				//FVector DrawEndPointLocation = RoadEndPosition + (JunctionPoints[i].RightVector * LeftAccumilatedWidth);

				//DebugPointPositions.Add(DrawStartPointLocation);
				//DrawDebugPoint(GetWorld(), DrawStartPointLocation, 10.0f, FColor::Blue, true, 1.0f, 2);

				//DebugPointPositions.Add(DrawEndPointLocation);
				//DrawDebugPoint(GetWorld(), DrawEndPointLocation, 50.0, FColor::Cyan, true, 5.0f, 2);

			}

			float RightAccumilatedWidth = 0.0f;

			for (int j = 0; j < JunctionPoints[i].RightLanes.Num(); j++)
			{
				RightAccumilatedWidth = RightAccumilatedWidth + JunctionPoints[i].RightLanes[j].LaneWidth;
				FVector DrawStartPointLocation = RoadStartPosition + (-JunctionPoints[i].RightVector * RightAccumilatedWidth);
				//FVector DrawEndPointLocation = RoadEndPosition + (-JunctionPoints[i].RightVector * RightAccumilatedWidth);

				DebugPointPositions.Add(DrawStartPointLocation);
				//DrawDebugPoint(GetWorld(), DrawStartPointLocation, 10.0f, FColor::Green, true, 1.0f, 2);

				//DebugPointPositions.Add(DrawEndPointLocation);
				//DrawDebugPoint(GetWorld(), DrawEndPointLocation, 50.0, FColor::Emerald, true, 5.0f, 2);

			}

		}
	}


}

//First Step is to generate all our key junctions Points - For each Incoming Road. Create three Points Where the junction bounds are
void AJunctionSurface::GenerateJunctionPoints()
{
	//Get our list of connected roads
	//For each road, loop through work out how many actual 'Incoming' sections we need 
	//Note - Our road points could be end points or middle points, so we need to solve what sort of junction this will be
	//For each 'Incoming' go and look into the road position and make the junction end point
	//Create our Edge Points at the Road Edges

	if (ConnectedRoads.Num() == 0)
	{
		return;
	}

	//Clear Points for now
	JunctionCenterPoints.Empty();
	JunctionVolumePoints.Empty();

	//For each of our Connected Roads find our junction end points and put them in an array
	for (int i = 0; i < ConnectedRoads.Num(); i++)
	{
		//UE_LOG(LogTemp, Warning, TEXT("MAKING JUNCTION ID: %d  At Section %d"), JunctionID, i);

		ARoadSurface* CurrentRoad = ConnectedRoads[i];
		const int CurrentPointID = ConnectedRoadPoints[i];
		bool isEndPoint = false;
		int PointCount = 2;
		int JunctionType = 1;
		const float Distance = 1000; //This is a temp harcoded value for testing,this is how far we should walk the spline to make our JunctionEnd
		float SplineDirection = 1.0f; //Should we walk forwards or backwards along the spline
		JunctionWorldLocation = this->GetTransform().GetLocation();

		//Get Current RoadSpline //May need to switch this to get by Tag
		USplineComponent* CurrentRoadSpline = CurrentRoad->GetComponentByClass<USplineComponent>();

		//Check if Point is at the End of a Curve or middle
		const float SplineLength = CurrentRoadSpline->GetSplineLength();
		const float JunctionDistanceAlongSpline = CurrentRoadSpline->GetDistanceAlongSplineAtSplinePoint(CurrentPointID);
		const int RoadTotalPointCount = CurrentRoadSpline->GetNumberOfSplinePoints();


		//If our PointID is equal to 0 or the end point on our RoadSpline it means the junction exists at the END of a road.
		if (CurrentPointID == 0 || CurrentPointID == (RoadTotalPointCount - 1))
		{
			//UE_LOG(LogTemp, Warning, TEXT("IS END POINT"));
			PointCount = 1;
			JunctionType = 0;

			//Should we walk forwards or backwards along spline
			if (CurrentPointID == (RoadTotalPointCount - 1))
			{
				SplineDirection = -1;
			}
		}

		//UE_LOG(LogTemp, Warning, TEXT("Junction Road: %d"), i);
		
		//Temporary Hold our Positions we have made
		TArray<FVector> PointPositions;

		//Create Positions for our Junction Points Center, Left and Right
		for (int j = 0; j < PointCount; j++)
		{
			//Temp - flip forwards backwards 
			if (j > 0)
			{
				SplineDirection = -SplineDirection;
			}
			
			UE_LOG(LogTemp, Warning, TEXT("INSIDE LOOP MAKING JCT: %d"), j);

			//Go and Get the Position along our spline for where to make the junction
			const float JunctionEndDistance = JunctionDistanceAlongSpline + (SplineDirection * Distance);
			const FVector JunctionStartLocation = CurrentRoadSpline->GetLocationAtDistanceAlongSpline(JunctionEndDistance, ESplineCoordinateSpace::World);
			
			//Create our Points to desribe the volume of our junction - these should exist along the edge of the road
			const FVector RightVector = CurrentRoadSpline->GetRightVectorAtDistanceAlongSpline(JunctionEndDistance, ESplineCoordinateSpace::World);
			const float RoadWidth = CurrentRoad->RoadWidth;
			
			//Add Our Location to Array
			PointPositions.Add(JunctionStartLocation);
			PointPositions.Add(JunctionStartLocation + (RightVector * RoadWidth));
			PointPositions.Add(JunctionStartLocation + (-RightVector * RoadWidth));

		}

		//UE_LOG(LogTemp, Warning, TEXT("FOR THIS SECTION FOUND: %d"), PointPositions.Num());

		//For these new Points add them to our struct array for processing later
		for(int j = 0; j < PointPositions.Num(); j++)
		{ 

			//Put our points into our Struct Array
			FJunctionPoint JunctionPoint;
			JunctionPoint.RoadID = i;
			JunctionPoint.Location = PointPositions[j];
			JunctionPoint.JunctionType = JunctionType;

			FVector JunctionPointCenter = PointPositions[j] - Location;
			JunctionPoint.AngleFromCenter = FMath::Atan2(JunctionPointCenter.Y, JunctionPointCenter.X);
		
			JunctionPoints.Add(JunctionPoint);
		}
	}

	
	JunctionPoints.Sort(FJunctionPoint());


}

//Draw our Junction Volume for Debugging //Sort All Points based on their angle to the center of the junction
void AJunctionSurface::DrawJunctionVolume()
{

	if (RoadManagerPoints.Num() == 0)
	{
		return; 
	}

	const int PointCount = RoadManagerPoints.Num();

	for (int i = 0; i < PointCount; i++)
	{
		DrawDebugPoint(GetWorld(), RoadManagerPoints[i].WorldSpacePosition, 10.0f, FColor::Blue, true, -1.0f, 10);

		int NextIndex = (i + 1) % PointCount;
		DrawDebugLine(GetWorld(), RoadManagerPoints[i].WorldSpacePosition , RoadManagerPoints[NextIndex].WorldSpacePosition, FColor::Cyan, true, -1.0f, 2.0f, 10.0f);
	}

}

//--Debug Draw Junction Shape RENAME THIS
void AJunctionSurface::DrawJunctionShape()
{
	if (JunctionCenterPoints.Num() != 0)
	{
		//FlushPersistentDebugLines(GetWorld());

		for (int i = 0; i < JunctionCenterPoints.Num(); i++)
		{
			DrawDebugPoint(GetWorld(), JunctionCenterPoints[i], 20.0f, FColor::Green, true, -1.0f, 10);
		}
	}
}

//- THis is for Debugging all the Points in the Junction
void AJunctionSurface::DrawRoadPoints()
{
	if (JunctionCenterPoints.Num() != 0)
	{
		
		//FlushPersistentDebugLines(GetWorld());

		for (int i = 0; i < JunctionCenterPoints.Num(); i++)
		{
			
			DrawDebugPoint(GetWorld(), JunctionCenterPoints[i], 20.0f, FColor::Red, true, -1.0f, 10);

		}
		
	}
}

// Called when the game starts or when spawned
void AJunctionSurface::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AJunctionSurface::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//A Lane Splne Creation
//These are modifications from the RoadSurface -> We need to unify all this
void AJunctionSurface::BuildAndUpdateLaneSplines()
{
	UE_LOG(LogTemp, Log, TEXT("Generated Lane Splines for traffic"));

	//Remove All Current Related Splines
	for (TSoftObjectPtr<ALaneSpline> CurrentLaneSpline : GeneratedLaneSplines)
	{
		TObjectPtr<ALaneSpline> LoadedSpline = CurrentLaneSpline.LoadSynchronous();

		if (LoadedSpline)
		{
			LoadedSpline->Destroy();
		}
	}

	GeneratedLaneSplines.Empty();

	for (int i = 0; i < JunctionPoints.Num(); i++)
	{
		//Get JunctionLength
		const int GeneratedResolution = ((JunctionPoints[i].Location - JunctionPoints[i].EndLocation).Size()) / DistanceBetweenPoints;

		FJunctionPoint CurrentJunction = JunctionPoints[i];

		//Generate all Left Lanes
		if (CurrentJunction.LeftLanes.Num() > 0)
		{
			for (int Lane = 0; Lane < CurrentJunction.LeftLanes.Num(); Lane++)
			{
				TArray<FSplinePoint> CurrentLanePoints = CreateLanePoints(GeneratedResolution, Lane, CurrentJunction.LeftLanes[Lane], CurrentJunction.LeftLanes, 0, (JunctionPoints[i].Location - JunctionPoints[i].EndLocation).Size(),JunctionPoints[i]);

				if (!CurrentLanePoints.IsEmpty())
				{
					//Spawn Lane Spline
					FTransform SpawnTransform;
					SpawnTransform.SetLocation(this->GetActorLocation());
					FActorSpawnParameters SpawnParams;

					TObjectPtr<ALaneSpline> NewLaneSpline = GetWorld()->SpawnActor<ALaneSpline>(SpawnParams);

					if (NewLaneSpline)
					{
						TObjectPtr<USplineComponent> SplineComponent = NewLaneSpline->LaneSpline;

						NewLaneSpline->SetActorLocation(SpawnTransform.GetLocation());
						SplineComponent->ClearSplinePoints();
						SplineComponent->AddPoints(CurrentLanePoints, true);

						GeneratedLaneSplines.Add(NewLaneSpline);
					}
				}
			}
		}

		//Generate all Left Lanes
		if (CurrentJunction.RightLanes.Num() > 0)
		{
			for (int Lane = 0; Lane < CurrentJunction.RightLanes.Num(); Lane++)
			{
				TArray<FSplinePoint> CurrentLanePoints = CreateLanePoints(GeneratedResolution, Lane, CurrentJunction.RightLanes[Lane], CurrentJunction.RightLanes, 1, (JunctionPoints[i].Location - JunctionPoints[i].EndLocation).Size(), JunctionPoints[i]);

				if (!CurrentLanePoints.IsEmpty())
				{
					//Spawn Lane Spline
					FTransform SpawnTransform;
					SpawnTransform.SetLocation(this->GetActorLocation());
					FActorSpawnParameters SpawnParams;

					TObjectPtr<ALaneSpline> NewLaneSpline = GetWorld()->SpawnActor<ALaneSpline>(SpawnParams);

					if (NewLaneSpline)
					{
						TObjectPtr<USplineComponent> SplineComponent = NewLaneSpline->LaneSpline;

						NewLaneSpline->SetActorLocation(SpawnTransform.GetLocation());
						SplineComponent->ClearSplinePoints();
						SplineComponent->AddPoints(CurrentLanePoints, true);

						GeneratedLaneSplines.Add(NewLaneSpline);
					}
				}
			}
		}
	}


	//Handle Junction / Turning Lanes
	CreateTurningLanePoints();

	//Create Splines
	//FJunctionTurningLane
	for (FJunctionTurningLane TurningLane : TurningLanes)
	{

		TArray<FSplinePoint> CurrenetTurningLanePoints = ConvertLocationsToSplinePoints(TurningLane.TurningLanePoints, -this->GetActorLocation());

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(this->GetActorLocation());
		FActorSpawnParameters SpawnParams;

		TObjectPtr<ALaneSpline> NewLaneSpline = GetWorld()->SpawnActor<ALaneSpline>(SpawnParams);
		NewLaneSpline->isJunctionLane = true;

		if (NewLaneSpline)
		{
			TObjectPtr<USplineComponent> SplineComponent = NewLaneSpline->LaneSpline;

			NewLaneSpline->SetActorLocation(SpawnTransform.GetLocation());
			SplineComponent->ClearSplinePoints();
			SplineComponent->AddPoints(CurrenetTurningLanePoints, true);
			SplineComponent->EditorUnselectedSplineSegmentColor = JunctionColorCodes[TurningLane.TurningLaneID];
			

			GeneratedLaneSplines.Add(NewLaneSpline);
		}
	}

}

TArray<FSplinePoint> AJunctionSurface::ConvertLocationsToSplinePoints(TArray<FVector> InLocations, FVector OffsetPoint)
{

	TArray<FSplinePoint> ReturnPoints;

	for (int i = 0; i < InLocations.Num(); i++)
	{
		FSplinePoint NewPoint;

		NewPoint.Position = InLocations[i] + OffsetPoint;
		NewPoint.InputKey = i;
		NewPoint.Scale = FVector(1, 1, 1);

		ReturnPoints.Add(NewPoint);
	}


	return ReturnPoints;
}


TArray<FSplinePoint> AJunctionSurface::CreateLanePoints(const int InResolution, const int LaneID, const FJunctionLaneData InLane, const TArray<FJunctionLaneData> InLaneArray, const int InLaneDirection, const float InJunctionLength, const FJunctionPoint InJunctionPoint)
{
	
	TArray<FSplinePoint> LanePoints;

	//const float SplineLength = InJunctionLength;
	float DistanceAlongSpline = 0.0f;
	const float IncrementDistance = 1.0 / InResolution;

	for (int i = 0; i < InResolution + 1; i++)
	{
		float TotalOffsetForLane = -InLaneArray[0].LaneWidth * 0.5f;
		float JunctionMaxDistance = (InJunctionPoint.CenterLinePoints[0].Location - InJunctionPoint.CenterLinePoints[InJunctionPoint.CenterLinePoints.Num() - 1].Location).Size();

		FVector CenterLinePointMaxDist = InJunctionPoint.CenterLinePoints[InJunctionPoint.CenterLinePoints.Num() - 1].Location;

		UE_LOG(LogTemp, Log, TEXT("Last Point of the CenterLine Location %s"), *CenterLinePointMaxDist.ToString());

		//Get our Offset across Lane Section <-- Handle Turning Lanes here
		for (int j = 0; j < LaneID + 1; j++)
		{
			TotalOffsetForLane += (InLaneArray[j].LaneWidth);
			UE_LOG(LogTemp, Log, TEXT("Offset for Lane at lane id: %i Width is: %f   Accumulated Width is %f"), j, InLaneArray[j].LaneWidth, TotalOffsetForLane);
		}

		if (InLaneDirection == 0)
		{
			//Forward Sample
			DistanceAlongSpline = FMath::Clamp(IncrementDistance * i, 0, 1.0);
		}
		else
		{
			//Backward Direction
			DistanceAlongSpline = 1.0 - FMath::Clamp(IncrementDistance * i, 0, 1.0);
		}

		UE_LOG(LogTemp, Log, TEXT("Junction Increment Distance %f"), DistanceAlongSpline);

		//Get Offset Position
		FVector OffsetPoint = FMath::Lerp(InJunctionPoint.Location + this->GetActorLocation(), InJunctionPoint.CenterLinePoints[InJunctionPoint.CenterLinePoints.Num() - 1].Location, DistanceAlongSpline);
		FVector OffsetVector = InJunctionPoint.RightVector.GetSafeNormal();

		FSplinePoint NewPoint;

		NewPoint.Position = (OffsetPoint - this->GetActorLocation() + (OffsetVector * TotalOffsetForLane));
		NewPoint.InputKey = i;
		NewPoint.Scale = FVector(1, 1, 1);

		LanePoints.Add(NewPoint);
	}

	return LanePoints;
}

void AJunctionSurface::CreateLaneSpline(const TArray<FVector> InSplinePoints)
{

}