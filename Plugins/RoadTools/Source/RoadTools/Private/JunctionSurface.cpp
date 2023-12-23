// Fill out your copyright notice in the Description page of Project Settings.


#include "JunctionSurface.h"
#include "ProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Math/Vector.h"

// Sets default values
AJunctionSurface::AJunctionSurface()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	JunctionSurface = CreateDefaultSubobject<UProceduralMeshComponent>("Road");
	JunctionSurface->SetupAttachment(GetRootComponent());

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
		UE_LOG(LogTemp, Warning, TEXT("MAKING JUNCTION ID: %d  At Section %d"), JunctionID, i);

		ARoadSurface* CurrentRoad = ConnectedRoads[i];
		const int CurrentPointID = ConnectedRoadPoints[i];
		bool isEndPoint = false;
		int PointCount = 2;
		int JunctionType = 1;
		const float Distance = 1000; //This is a temp harcoded value for testing,this is how far we should walk the spline to make our JunctionEnd
		float SplineDirection = 1.0f; //Should we walk forwards or backwards along the spline
		const FVector JunctionWorldLocation = this->GetTransform().GetLocation();

		//Get Current RoadSpline //May need to switch this to get by Tag
		USplineComponent* CurrentRoadSpline = CurrentRoad->GetComponentByClass<USplineComponent>();

		//Check if Point is at the End of a Curve or middle
		const float SplineLength = CurrentRoadSpline->GetSplineLength();
		const float JunctionDistanceAlongSpline = CurrentRoadSpline->GetDistanceAlongSplineAtSplinePoint(CurrentPointID);
		const int RoadTotalPointCount = CurrentRoadSpline->GetNumberOfSplinePoints();


		//If our PointID is equal to 0 or the end point on our RoadSpline it means the junction exists at the END of a road.
		if (CurrentPointID == 0 || CurrentPointID == (RoadTotalPointCount - 1))
		{
			UE_LOG(LogTemp, Warning, TEXT("IS END POINT"));
			PointCount = 1;
			JunctionType = 0;

			//Should we walk forwards or backwards along spline
			if (CurrentPointID == (RoadTotalPointCount - 1))
			{
				SplineDirection = -1;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Junction Road: %d"), i);
		
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

		UE_LOG(LogTemp, Warning, TEXT("FOR THIS SECTION FOUND: %d"), PointPositions.Num());

		//For these new Points add them to our struct array for processing later
		for(int j = 0; j < PointPositions.Num(); j++)
		{ 

			//Put our points into our Struct Array
			FJunctionPoint JunctionPoint;
			JunctionPoint.RoadID = i;
			JunctionPoint.Position = PointPositions[j];
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

