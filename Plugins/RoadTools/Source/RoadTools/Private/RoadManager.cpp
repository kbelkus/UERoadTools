// Fill out your copyright notice in the Description page of Project Settings.
#include "RoadManager.h"
#include "Engine/World.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
ARoadManager::ARoadManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}


void ARoadManager::OnConstruction(const FTransform& RootTransform)
{

	UWorld* World = GetWorld(); // Get the current world

	FlushPersistentDebugLines(GetWorld());

	if (UpdateRoads == true)
	{
		GatherRoads(World);
		UpdateRoads = false;

		GetAllCurvePoints();

		SolveIntersections();
		SolveIntersectionKeyPoints();

		DebugDrawIntersections();


		//Rebuild Junctions if Needed
		if (JunctionCollection.Num() > 0)
		{
			for (int i = 0; i < JunctionCollection.Num(); i++)
			{
				//CHeck for Nullptr here
				JunctionCollection[i]->Destroy();
			}

			JunctionCollection.Empty();
		}
	
		CreateJunctions(World);

		UE_LOG(LogTemp, Warning, TEXT("Rebuilding Road Network"));

		UpdateRoadSurfaces();
	}

}



//Get all the Roads in the scene and stick them in array
void ARoadManager::GatherRoads(UWorld* World)
{
	RoadCollection.Empty();

	TArray<AActor*> SceneActors;
	UGameplayStatics::GetAllActorsOfClass(World, ARoadSurface::StaticClass(), SceneActors);

	int SplinePartsCount;

	for (int i = 0; i < SceneActors.Num(); i++)
	{
		//RoadCollection.Add(SceneActors[i]);


		//Get Component
		USplineComponent* RoadSpline;
		RoadSpline = SceneActors[i]->GetComponentByClass<USplineComponent>();

		SplinePartsCount = RoadSpline->GetNumberOfSplineSegments();


		UE_LOG(LogTemp, Warning, TEXT("Spline Parts:  %d"), SplinePartsCount);

		//Build our Array of all Scene Roads
		ARoadSurface* CurrentRoadSurface = Cast<ARoadSurface>(SceneActors[i]);
		RoadCollection.Add(CurrentRoadSurface);

	}

	UE_LOG(LogTemp, Warning, TEXT("Found Roads:  %d"), SceneActors.Num());

}


//Check scene for all junction actors
void ARoadManager::GatherJunctions(UWorld* World)
{
	JunctionCollection.Empty();

	TArray<AActor*> SceneJunctions;
	UGameplayStatics::GetAllActorsOfClass(World, ARoadSurface::StaticClass(), SceneJunctions);

	for (int i = 0; i < SceneJunctions.Num(); i++)
	{
		AJunctionSurface* CurrentJunctionSurface = Cast<AJunctionSurface>(SceneJunctions[i]);
	}
}


//This will create junction actors in the world - and check that existing actors are still valid 
void ARoadManager::CreateJunctions(UWorld* World)
{
	if (JunctionConstructionData.Num() == 0)
	{
		return;
	}


	JunctionCollection.Empty();

	const int JunctionCount = JunctionConstructionData.Num();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//Create our Junctions
	for (int i = 0; i < JunctionCount; i++)
	{

		UE_LOG(LogTemp, Warning, TEXT("MAKING JUNCTION : %d"), i);

		//Get Data for this junction

		FJunctionStructure CurrentJunction;

		CurrentJunction = JunctionConstructionData[i];
		
		FVector WorldLocation = CurrentJunction.WorldPosition;
		FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);

		AJunctionSurface* NewJunction = World->SpawnActor<AJunctionSurface>(AJunctionSurface::StaticClass(), WorldLocation, SpawnRotation, SpawnParams);

		//Set New Junction Data
		if (NewJunction)
		{
			UE_LOG(LogTemp, Warning, TEXT("Created new Junction: %d"), i);

			const FString Name = "Junction" + FString::FromInt(i);

			NewJunction->SetActorLabel(Name);
			NewJunction->ConnectedRoads = CurrentJunction.ConnectedRoad;
			
			//Set Point Struct Data -- This is basically converting the poitns we made in the Road Manager to the Junction Surface struct, 
			for (int j = 0; j < CurrentJunction.RoadPoints.Num(); j++)
			{
				FPointsFromRoadManager Points;
				Points.ConnectedRoad = CurrentJunction.RoadPoints[j].ConnectedRoad;
				Points.WorldSpaceNormal = CurrentJunction.RoadPoints[j].WorldSpaceNormal;
				Points.WorldSpacePosition = CurrentJunction.RoadPoints[j].WorldSpacePosition;
				Points.WorldSpaceRightVector = CurrentJunction.RoadPoints[j].WorldSpaceRightVector;

				NewJunction->RoadManagerPoints.Add(Points);
			}

			NewJunction->JunctionID = i;
			JunctionCollection.Add(NewJunction);

			NewJunction->BuildJunction();
		}
	}
}


//Run this function to update RoadSurfaces when they have junction data
void ARoadManager::UpdateRoadSurfaces()
{

	if (RoadIntersections.IsEmpty())
	{
		return;
	}



	//Go through array and set data to each road surface.
	//We also need to set if the first road point is a junction or not
	//Otherwise we dont know, where to start generating surfaces from
	for (const TPair<ARoadSurface*, FRoadSurfaceIntersectPoint> RoadData : RoadIntersections)
	{
		RoadData.Key;
		RoadData.Value;

		//Loop through or Points Array and make a list of all the points in our road curve that are associated with an intersection

		TArray<int> FoundCurvePoints;

		for (int i = 0; i < RoadJunctionPointIDs.Num(); i++)
		{
			if (RoadData.Key == RoadJunctionPointIDs[i].RoadSurface)
			{
				FoundCurvePoints.Add(RoadJunctionPointIDs[i].CurvePointID);
			}
		}


		RoadData.Key->IntersectPoints = RoadData.Value.DistanceAlongCurve;
		RoadData.Key->JunctionPoints = FoundCurvePoints;
		RoadData.Key->BuildRoad();

	}
}


//Solve points where the junctions intersects the road.
//This creates the keypoints which we can use pass to the road and the junction surfaces so we can control where the geometry gets created.
//Pass these as positions to each road 
void ARoadManager::SolveIntersectionKeyPoints()
{

	if (JunctionData.Num() == 0)
	{
		return;
	}

	int index = 0;

	JunctionConstructionData.Empty();
	RoadIntersections.Empty();

	//For Each Junction
	for (const TPair<FVector, FRoadIntersections>& SampledJunction : JunctionData)
	{
		SampledJunction.Key;
		FRoadIntersections CurrentJunction =  SampledJunction.Value;

		const FVector JunctionCenter = CurrentJunction.IntersectPosition;
		TArray<FVector> JunctionCenterPoints;
		TArray<FRoadPoint> JunctionPoints;
		TArray<FRoadPoint> RoadPoints;


		//For this Junction, now solve where the junction intersects the road and make the required points
		//Go Through Road by Road
		for (int i = 0; i < CurrentJunction.RoadReference.Num(); i++)
		{

			//For this Road, Get the Road, Spline and Point that make up this junction
			ARoadSurface* CurrentRoad = CurrentJunction.RoadReference[i];
			const int CurrentPointID = CurrentJunction.PointID[i];
			int PointCount = 2; //How Many Points to make for this intersection
			int JunctionType = 1; //Junction Type 
			const float Distance = 1000; //This is a temp harcoded value for testing,this is how far we should walk the spline to make our JunctionEnd
			float SplineDirection = 1.0f; //Should we walk forwards or backwards along the spline
			
			//Solve Road Width (Move this to a function later since with can vary over road surface
			float CurrentRoadWidth = CurrentRoad->RoadWidth;

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

			//Create our Junction Points, Add to this Junction Point Array to our Main Junction Construction Data
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
				const FVector JunctionNormal = CurrentRoadSpline->GetDirectionAtDistanceAlongSpline(JunctionEndDistance, ESplineCoordinateSpace::World);
				const FVector JunctionRightVector = CurrentRoadSpline->GetRightVectorAtDistanceAlongSpline(JunctionEndDistance, ESplineCoordinateSpace::World);

				//Create our Point Assign it to array for this junction
				FRoadPoint NewPointToAdd;

				NewPointToAdd.WorldSpacePosition = JunctionStartLocation;
				NewPointToAdd.WorldSpaceNormal = JunctionNormal;
				NewPointToAdd.WorldSpaceRightVector = JunctionRightVector;
				NewPointToAdd.ConnectedRoad = CurrentRoad;
				
				JunctionPoints.Add(NewPointToAdd);

				//Add Edge Boundary Points
				FRoadPoint LeftPoint;
				LeftPoint.WorldSpacePosition = JunctionStartLocation + (JunctionRightVector * CurrentRoadWidth);
				LeftPoint.WorldSpaceNormal = JunctionNormal;
				LeftPoint.WorldSpaceRightVector = JunctionRightVector;
				LeftPoint.ConnectedRoad = CurrentRoad;
				JunctionPoints.Add(LeftPoint);

				FRoadPoint RightPoint;
				RightPoint.WorldSpacePosition = JunctionStartLocation + (-JunctionRightVector * CurrentRoadWidth);
				RightPoint.WorldSpaceNormal = JunctionNormal;
				RightPoint.WorldSpaceRightVector = JunctionRightVector;
				RightPoint.ConnectedRoad = CurrentRoad;
				JunctionPoints.Add(RightPoint);

				//Build Array of Roads and their points
				FRoadSurfaceIntersectPoint NewRoadPoint;

				TArray<float> CurrentDistances;

				FRoadSurfaceIntersectPoint* ValuePtr = RoadIntersections.Find(CurrentRoad);

				if (ValuePtr != nullptr)
				{

					CurrentDistances = ValuePtr->DistanceAlongCurve;

					CurrentDistances.Add(JunctionEndDistance);

					FRoadSurfaceIntersectPoint UpdatedData;
					UpdatedData.DistanceAlongCurve = CurrentDistances;

					RoadIntersections.Add(CurrentRoad, UpdatedData);

				}
				else
				{
					FRoadSurfaceIntersectPoint NewPointData;
					NewPointData.DistanceAlongCurve.Add(JunctionEndDistance);


					RoadIntersections.Add(CurrentRoad, NewPointData);

				}

			}

		}

		//Make our Junction Data -- This is the data that will be passed to the Junction GameObject so it can build itself
		FJunctionStructure NewJunction;

		//Assign Points
		NewJunction.RoadPoints.Append(JunctionPoints);
		NewJunction.JunctionID = index;
		NewJunction.ConnectedRoad.Append(CurrentJunction.RoadReference);
		NewJunction.WorldPosition = JunctionCenter;


		JunctionConstructionData.Add(NewJunction);
		

		index = index + 1;


	}

}


void CreateJunctionObject(UWorld* World)
{
	UE_LOG(LogTemp, Warning, TEXT("Making Junction"));

}


//Go through Roads and find where all the intersections actually exist
void ARoadManager::SolveIntersections()
{
	if (RoadCollection.Num() == 0 && CurvePoints.Num() == 0)
	{
		return;
	}

	Intersections.Empty();
	JunctionData.Empty();
	RoadJunctionPointIDs.Empty();

	//For Each Point we captured, go through and find if there is ANY points within the roughly the same distance
	//Optimise this later, by doing it via grid cells
	//Find a method to not check points in its own curve... since it will now - Maybe store length into the struct
	TArray<FVector> FinalJunctionPositions;
	
	for (int i = 0; i < CurvePoints.Num(); i++)
	{
		FVector Positions = FVector(0.0f, 0.0f, 0.0f);
		FVector P0Position = CurvePoints[i].PointPosition;

		FRoadIntersections IIntersections;

		TArray<ARoadSurface*> FoundRoadReference;
		TArray<int> FoundRoadID;
		TArray<int> FoundPointID;
		int FoundMatches = 0;

		Positions = P0Position;

		for (int j = 0; j < CurvePoints.Num(); j++)
		{

			if (i == j)
			{
				continue;
			}

			FVector P1Position = CurvePoints[j].PointPosition;

			//Distance Check - Add our elements to our struct here...
			if (FVector::Distance(P1Position, P0Position) < 1000.0f)
			{

				//Get Road Reference
				Positions = Positions + P1Position;

				int RoadID = CurvePoints[j].RoadReference;
				ARoadSurface* RoadReference = RoadCollection[RoadID];
				FoundRoadReference.Add(RoadReference);
				FoundRoadID.Add(RoadID);

				//Get the ID of the point WITHIN the road.
				int RealPointID = CurvePoints[j].PointID;
				FoundPointID.Add(RealPointID); //Is this also wrong?

				FoundMatches += 1;


				//Make an array here, so we can easily sample this later and set to each road
				FPointOnRoadCurveJunction NewRoadData;

				NewRoadData.RoadSurface = RoadReference;
				NewRoadData.CurvePointID = RealPointID;
				
				RoadJunctionPointIDs.Add(NewRoadData);

				
			}
		}


		int FinalPositionsCount = FinalJunctionPositions.Num();

		//Append all our Data to our Arrays - include first iterator as the root
		if (FoundMatches != 0)
		{
			int RoadID = CurvePoints[i].RoadReference;
			ARoadSurface* RoadReference = RoadCollection[RoadID];
			FoundRoadReference.Add(RoadReference);
			FoundRoadID.Add(RoadID);
			int RealPointID = CurvePoints[i].PointID;
			FoundPointID.Add(RealPointID); //This is wrong -- We need to get the point ID relative to the actual road.

			FVector IntersectionPosition = Positions / (FoundMatches + 1);
			IIntersections.IntersectPosition = IntersectionPosition;
			IIntersections.RoadReference = FoundRoadReference;
			IIntersections.RoadID = FoundRoadID;
			IIntersections.PointID = FoundPointID;
			
			JunctionData.Add(IntersectionPosition, IIntersections);

			//Also Assign This Data back to the road as a simple Array so the Road is aware of which points
			//Are being used as junction points


		}
	}
}


//Go through all our curves and put all points in a big array, so we can check everything
void ARoadManager::GetAllCurvePoints()
{

	if (RoadCollection.Num() == 0)
	{
		return;
	}

	CurvePoints.Empty();

	for (int i = 0; i < RoadCollection.Num(); i++)
	{		
		USplineComponent* RoadSpline;
		RoadSpline = RoadCollection[i]->GetComponentByClass<USplineComponent>();

		int PointCount = RoadSpline->GetNumberOfSplinePoints();

		for (int j = 0; j < PointCount; j++)
		{

			FCurvePoints ICurvePoint;

			ICurvePoint.PointPosition = RoadSpline->GetLocationAtSplineInputKey(j, ESplineCoordinateSpace::World);
			ICurvePoint.PointID = j;
			ICurvePoint.RoadReference = i;

			CurvePoints.Add(ICurvePoint);

		}
	}
}


void ARoadManager::DebugDrawIntersections()
{
	if (JunctionData.Num() > 0)
	{

		int counter = 0;

		for (const TPair<FVector, FRoadIntersections>& pair : JunctionData)
		{
			pair.Key;
			pair.Value;

			FVector TempLocation = FVector(100, 100, 100);

			DrawDebugSphere(GetWorld(), pair.Value.IntersectPosition, 100.0f, 32, FColor::Blue, true, -1.0f, 2.0f, 1.0f);
			UE_LOG(LogTemp, Warning, TEXT("RenderingSphere:  %d"), counter);

			counter = counter + 1;

		}
	}
}



// Called when the game starts or when spawned
void ARoadManager::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("GAME RUNNING"));

	int counter = 0;

	for (const TPair<FVector, FRoadIntersections>& pair : JunctionData)
	{
		pair.Key;
		pair.Value;

		DrawDebugString(GetWorld(), pair.Key + FVector(0.0f, 0.0f, counter * 100.0f), "JctID" + counter, nullptr, FColor::Red, -1.0, false, 5.0f);


		counter += counter;

	}


}

// Called every frame
void ARoadManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);




	UE_LOG(LogTemp, Warning, TEXT("Im Looping"));



}

