// Fill out your copyright notice in the Description page of Project Settings.


#include "SidewalkSurface.h"
#include "JunctionSurface.h"
#include "RoadSurface.h"
#include "ProceduralMeshComponent.h"

// Sets default values
ASidewalkSurface::ASidewalkSurface()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	SetRootComponent(RootComponent);

	PrimaryEdgeSpline = CreateDefaultSubobject<USplineComponent>("PrimaryEdgeSpline");
	PrimaryEdgeSpline->SetupAttachment(RootComponent);

	SecondaryEdgeSpline = CreateDefaultSubobject<USplineComponent>("SecondaryEdgeSpline");
	SecondaryEdgeSpline->SetupAttachment(RootComponent);

	SidewalkPrimarySurface = CreateDefaultSubobject<UProceduralMeshComponent>("SidewalkPrimarySurface");
	SidewalkPrimarySurface->SetupAttachment(RootComponent);

	InitialiseSideWalkProperties();
	
}

void ASidewalkSurface::OnConstruction(const FTransform& Transform)
{ 
	Super::OnConstruction(Transform);

	RebuildSideWalkGeometry();

}

void ASidewalkSurface::RebuildSideWalkSurface()
{
	bool bCanRebuildGeometry = false;

	bCanRebuildGeometry = UpdatePrimaryEdgeSpline();

	if (bCanRebuildGeometry)
	{
		UE_LOG(LogTemp, Log, TEXT("From here, lets rebuild all our glorious sidewalk geo"));

		RebuildSideWalkGeometry();
	}
}

bool ASidewalkSurface::UpdatePrimaryEdgeSpline()
{
	bool bHandled = false;

	if(SideWalkGenerationType == ESideWalkGenerationType::ROAD)
	{
		bHandled = GetRoadSurfaceEdge();
	}

	return bHandled;
}

bool ASidewalkSurface::GetRoadSurfaceEdge()
{
	bool bHandled = false;

	if (!ConnectedRoadSurface)
	{
		return bHandled;
	}

	TObjectPtr<USplineComponent> RoadSplineComponent = ConnectedRoadSurface->CenterSpline;
	PrimaryEdgeSpline->ClearSplinePoints();

	//Copy the spline Points from this Spline into our Primary Spline with the road offset
	TArray<FVector> RoadSplinePoints;
	TArray<FVector> TangentAtSplinePoints;

	const int StepCount = 20;
	const float Step = RoadSplineComponent->GetSplineLength() / StepCount;

	float AccumilatedRoadWidth = 0.0f;
	//Solve Road Width 

	if (SideWalkGenerationSide == ESideWalkGenerationSide::BOTH)
	{
		return bHandled;
	}

	if (SideWalkGenerationSide == ESideWalkGenerationSide::LEFT)
	{
		for (FLaneData Lane : ConnectedRoadSurface->LeftLanes)
		{
			AccumilatedRoadWidth += Lane.LaneWidth;
		}
	}

	if (SideWalkGenerationSide == ESideWalkGenerationSide::RIGHT)
	{
		for (FLaneData Lane : ConnectedRoadSurface->RightLanes)
		{
			AccumilatedRoadWidth += Lane.LaneWidth;
		}
	}

	for (int i = 0; i < StepCount + 1; i++)
	{
		float IncrementDistance = i * Step;
		//Get Spline Point at Distance Along Spline
		float DistanceAlongSpline = RoadSplineComponent->GetDistanceAlongSplineAtSplinePoint(IncrementDistance);

		FVector PointLocation = (RoadSplineComponent->GetLocationAtDistanceAlongSpline(IncrementDistance, ESplineCoordinateSpace::World) + (RoadSplineComponent->GetRightVectorAtDistanceAlongSpline (IncrementDistance, ESplineCoordinateSpace::World) * AccumilatedRoadWidth));

		PrimaryEdgeSpline->AddSplineWorldPoint(PointLocation);
	}

	bHandled = true;
	return bHandled;
}

void ASidewalkSurface::GetJunctionEdge()
{

}

void ASidewalkSurface::InitialiseSideWalkProperties()
{
	//Get Connected Actors and find what type we need to connect to and set our properties.
	if (!ConnectedSurface)
	{
		UE_LOG(LogTemp, Error, TEXT("SideWalkSurface::InitialiseSideWalkProperties - no connected surface. Please add one. Will exit"));
		SideWalkGenerationType = ESideWalkGenerationType::NONE;
		return;
	}

	if (ConnectedSurface.IsA(AJunctionSurface::StaticClass()))
	{
		SideWalkGenerationType = ESideWalkGenerationType::JUNCTION;
	}

	if (ConnectedSurface.IsA(ARoadSurface::StaticClass()))
	{
		SideWalkGenerationType = ESideWalkGenerationType::ROAD;
		ConnectedRoadSurface = Cast<ARoadSurface>(ConnectedSurface);
	}
}

void ASidewalkSurface::RebuildSideWalkGeometry()
{
	SidewalkPrimarySurface->ClearMeshSection(0);

	bool LeftRightFlip = false;

	switch (SideWalkGenerationSide)
	{
	case ESideWalkGenerationSide::LEFT:
		LeftRightFlip = false;
		break;
	case ESideWalkGenerationSide::RIGHT:
		LeftRightFlip = true;
		break;
	case ESideWalkGenerationSide::BOTH:
		return;
		break;
	default:
		break;
	}

	//Build Indices Buffer
	FVertexBufferSimple SideWalkBuffer = BuildMeshGridFromSpline(PrimaryEdgeSpline, LengthResolution, WidthResolution, 0.0f, LeftRightFlip, SideWalkWidth);
	TArray<int> TriangleIndices = BuildTriangleIndices(LengthResolution, WidthResolution, LeftRightFlip);

	SidewalkPrimarySurface->CreateMeshSection(0, SideWalkBuffer.Location, TriangleIndices, SideWalkBuffer.Normal, SideWalkBuffer.UV, TArray<FColor>(), TArray<FProcMeshTangent>(), true);

	//Debug Draw
	DebugDrawPositions = SideWalkBuffer.Location;

	UWorld* WorldContext = GetWorld();

	FlushPersistentDebugLines(WorldContext);

	for (int i = 0; i < DebugDrawPositions.Num(); i++)
	{
		DrawDebugPoint(WorldContext, DebugDrawPositions[i] + this->GetActorLocation(), 5.0f, FColor::Blue, true, -1.0f, 2);
	}
}

FVertexBufferSimple ASidewalkSurface::BuildMeshGridFromSpline(USplineComponent* InSplineComponent, int InXResolution, int InYResolution, float OffsetY, bool ToggleLeftRight, float InWidth)
{
	const float Width = ToggleLeftRight ? InWidth : -InWidth;

	const float XStepValue = InSplineComponent->GetSplineLength() / InXResolution;
	const float YStepValue = Width / InYResolution;

	FVertexBufferSimple ReturnVertexBuffer = FVertexBufferSimple();

	TArray<FVector> Locations = TArray<FVector>();
	TArray<FVector> Normals = TArray<FVector>();
	TArray<FVector2D> UVs = TArray<FVector2D>();

	for (int i = 0; i < InXResolution + 1; i++)
	{
		FVector SplineStartPoint = InSplineComponent->GetWorldLocationAtDistanceAlongSpline(XStepValue * i);

		for (int j = 0; j < InYResolution + 1; j++)
		{
			FVector PointAlongWidth = SplineStartPoint + (InSplineComponent->GetRightVectorAtDistanceAlongSpline(XStepValue * i, ESplineCoordinateSpace::World) * (j * YStepValue));

			Locations.Add(PointAlongWidth  - this->GetActorLocation());
			Normals.Add(FVector(0, 0, 1));
			UVs.Add(FVector2D(XStepValue * i, YStepValue * j));
		}
	}

	ReturnVertexBuffer.Location = Locations;
	ReturnVertexBuffer.Normal = Normals;
	ReturnVertexBuffer.UV = UVs;

	return ReturnVertexBuffer;
}

TArray<int> ASidewalkSurface::BuildTriangleIndices(int InXResolution, int InYResolution, bool ReverseWindingOrder)
{
	TArray<int> Triangles = TArray<int>();

	const int TriangleCount = (InXResolution - 1) * (InYResolution - 1);

	int RootIndex = 0;

	if (ReverseWindingOrder == false)
	{
		for (int i = 0; i < InXResolution; i++)
		{
			RootIndex = i * (InYResolution + 1);

			//For Each Row
			for (int j = 0; j < InYResolution; j++)
			{
				int FirstIndex = RootIndex + j;
				int SecondIndex = (RootIndex + j) + 1;
				int ThirdIndex = (RootIndex + j) + (InYResolution + 1);

				Triangles.Add(FirstIndex);
				Triangles.Add(ThirdIndex);
				Triangles.Add(SecondIndex);

				int FourthIndex = (RootIndex + j) + 1;
				int FifthIndex = (RootIndex + j) + (InYResolution + 2);
				int SixthIndex = (RootIndex + j) + (InYResolution + 1);

				Triangles.Add(FourthIndex);
				Triangles.Add(SixthIndex);
				Triangles.Add(FifthIndex);
			}
		}
	}
	else
	{
		for (int i = 0; i < InXResolution; i++)
		{
			RootIndex = i * (InYResolution + 1);

			//For Each Row
			for (int j = 0; j < InYResolution; j++)
			{
				int FirstIndex = RootIndex + j;
				int SecondIndex = (RootIndex + j) + 1;
				int ThirdIndex = (RootIndex + j) + (InYResolution + 1);

				Triangles.Add(FirstIndex);
				Triangles.Add(SecondIndex);
				Triangles.Add(ThirdIndex);

				int FourthIndex = (RootIndex + j) + 1;
				int FifthIndex = (RootIndex + j) + (InYResolution + 2);
				int SixthIndex = (RootIndex + j) + (InYResolution + 1);

				Triangles.Add(FourthIndex);
				Triangles.Add(FifthIndex);
				Triangles.Add(SixthIndex);
			}
		}
	}

	return Triangles;
}
