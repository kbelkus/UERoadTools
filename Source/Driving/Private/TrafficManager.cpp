// Fill out your copyright notice in the Description page of Project Settings.

#include "TrafficManager.h"
#include "Kismet/GameplayStatics.h"
#include "LaneSpline.h"
#include "TrafficGridObject.h"
#include "Math/UnrealMathUtility.h"
#include "GenericPlatform/GenericPlatformMath.h"

// Sets default values
ATrafficManager::ATrafficManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}



void ATrafficManager::OnConstruction(const FTransform& RootTransform)
{

	if (RebuildGrid == true)
	{
		CreateTrafficGrid();
		RebuildGrid = false;
	}

}


//Get all the lanes near the player (Later cache this in a road manager and divide world into grid so we can sort / retrieve roads by querying one object
void ATrafficManager::GetLanes()
{

	AvailableRoads.Empty();
	TArray<AActor*> SceneActors;
	
	//Get all the roads in the scene (move this to grid system later)

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALaneSpline::StaticClass(), SceneActors);

	for (AActor* Actor : SceneActors)
	{

		ALaneSpline* FoundLane = Cast<ALaneSpline>(Actor);
		if (FoundLane)
		{
			if (FoundLane->isJunctionLane == false)
			{
				FLanes FoundLaneSpline;

				//Get Spline Center Point
				USplineComponent* LaneSplineComponent;
				LaneSplineComponent = FoundLane->GetComponentByClass<USplineComponent>();
				FVector FirstPoint = LaneSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
				FVector LastPoint = LaneSplineComponent->GetLocationAtSplinePoint(LaneSplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);

				FoundLaneSpline.Lane = FoundLane;
				
				FoundLaneSpline.LanePosition = (FirstPoint + LastPoint) * 0.5f;
				AvailableRoads.Add(FoundLaneSpline);

			}
		}
	}
}

//Spawn all traffic agents on begin play
void ATrafficManager::InitialiseTrafficAgents()
{

	if (TrafficLibrary.Num() == 0 || TrafficAgentsPoolSize == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Traffic Manager: Initialise Traffic Agents - Could not spawn traffic either library is empty or agent count is 0"));
		return;
	}

	int Cycle = 0;

	for (int i = 0; i < TrafficAgentsPoolSize; i++)
	{
		//Spawn our vehicles on random lanes

		FTrafficAgents TrafficAgent;
		//ABaseTrafficVehicle* TrafficVehicle;
		//Get Random Road (CHECK IF SPAWNING AT SAME PLACE AS ANOTHER ACTOR)
		int RoadIndex = i % (AvailableRoads.Num() - 1);

		if (RoadIndex == AvailableRoads.Num() - 1)
		{
			Cycle = Cycle + 1;
		}

		float PositionAlongSpline = Cycle * 0.10f; //Fixed Number for now, because we will never spawn that many vehicles

		//Init Properties
		USplineComponent* SpawnSpline;
		SpawnSpline = AvailableRoads[RoadIndex].Lane->ReturnSpline();

		FVector SpawnLocation = SpawnSpline->GetLocationAtDistanceAlongSpline(PositionAlongSpline, ESplineCoordinateSpace::World);
		FRotator SpawnRotation = SpawnSpline->GetRotationAtDistanceAlongSpline(PositionAlongSpline, ESplineCoordinateSpace::World);

		//Spawn Agent
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		ABaseTrafficVehicle* NewTrafficAgent = GetWorld()->SpawnActor<ABaseTrafficVehicle>(ABaseTrafficVehicle::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);

		if (NewTrafficAgent)
		{
			NewTrafficAgent->CurrentLane = AvailableRoads[RoadIndex].Lane;
			NewTrafficAgent->PositionAlongSplineLength = 0.0;
			NewTrafficAgent->VehicleSpeed = 800.0f;
			NewTrafficAgent->VehicleMesh->SetStaticMesh(TrafficLibrary[0]->VehicleMesh->GetStaticMesh());
			NewTrafficAgent->SetSpawnLocation();	
		}

		//TrafficAgent
		TrafficAgent.DistanceFromPlayer = FVector::Distance(PlayerWorldLocation, SpawnLocation);
		TrafficAgent.TrafficAgent = NewTrafficAgent;
		TrafficAgent.delta = 0;
		TrafficAgent.PositiveDeltaCount = 0;

		//Add Traffic Agent to Manager Pool
		TrafficAgentsPool.Add(TrafficAgent);

	}


}

//Update the agent struct and mark any agents for respawning/moving
void ATrafficManager::UpdateTrafficAgents()
{

	//For all our agents loop through and check thier positions
	const int VehicleCount = TrafficAgentsPool.Num();

	TrafficAgentRespawnList.Empty();

	//Get Vehicle Location
	//Get Distance from Player
	//if is outside of threshhold mark it
	//if is already marked get previous distance and mark the delta

	for (int i = 0; i < VehicleCount - 1; i++)
	{

		FVector VehicleLocation;

		//Get Distance
		float DistanceToPLayer = FVector::Distance(TrafficAgentsPool[i].TrafficAgent->GetActorLocation(), PlayerWorldLocation);
		float delta = DistanceToPLayer - TrafficAgentsPool[i].PreviousDistanceFromPlayer;

		//If our Vehicle is outside of our range
		if (DistanceToPLayer > MaxVehicleDistance && delta > 0)
		{
			TrafficAgentsPool[i].PositiveDeltaCount = TrafficAgentsPool[i].PositiveDeltaCount + 1;
			TrafficAgentsPool[i].PreviousDistanceFromPlayer = DistanceToPLayer;
		}
		else
		{
			TrafficAgentsPool[i].delta = 0;
			TrafficAgentsPool[i].PositiveDeltaCount = 0;
			TrafficAgentsPool[i].TrafficAgent->MarkedForRespawn = false;
		}

		if (TrafficAgentsPool[i].PositiveDeltaCount > 3)
		{
			UpdateSingleTrafficAgent(TrafficAgentsPool[i]);
			TrafficAgentRespawnList.Add(i);
		}

	}

}



//Respawn a single agent when needed
void ATrafficManager::UpdateSingleTrafficAgent(FTrafficAgents TrafficAgent)
{
	//Set Something to the traffic Agent here
	TrafficAgent.TrafficAgent->MarkedForRespawn = true;
}



void ATrafficManager::RespawnTrafficAgents()
{

	if (CurrentSpawnableLanes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Spawnable Lanes"))
		return;
	}

	//For Each Agent in our list, find a new road location and update it
	for (int i = 0; i < TrafficAgentRespawnList.Num(); i++)
	{
		//Loop through each aviable cell so we spawn cars in some linear order
		//int CellIindex = i % ActiveSpawnCells.Num();
		bool CanSpawn = false;

		//Randomise Lane Selection
		int RandomLaneID = GetRandomInt(0, CurrentSpawnableLanes.Num() - 1);
		UE_LOG(LogTemp, Warning, TEXT("RandomLaneID, %i"), RandomLaneID);

		ABaseTrafficVehicle* CurrentAgent = TrafficAgentsPool[TrafficAgentRespawnList[i]].TrafficAgent;

		USplineComponent* SpawnSpline;
		//int LaneIndex = i % (CurrentSpawnableLanes.Num() - 1);
		SpawnSpline = CurrentSpawnableLanes[RandomLaneID]->ReturnSpline(); //CLAMP THIS

		//This is fixed, but we want to eventually async check along road
		float SplineTime = 0.1f;

		FVector SpawnLocation = SpawnSpline->GetWorldLocationAtTime(SplineTime, false);
		float DistanceAlongSpline = SpawnSpline->GetSplineLength() * SplineTime;

		//Eventually move this to a re-init function
		if (SpawnSpline)
		{
			CurrentAgent->SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
			CurrentAgent->CurrentLane = CurrentSpawnableLanes[RandomLaneID];
			CurrentAgent->CurrentSpline = SpawnSpline;
			CurrentAgent->PositionAlongSplineLength = DistanceAlongSpline;
			CurrentAgent->CurrentLaneLength = SpawnSpline->GetSplineLength();
			CurrentAgent->MarkedForRespawn = false;
			CurrentAgent->FoundNextLane = false;
			TrafficAgentsPool[TrafficAgentRespawnList[i]].delta = 0.0;
			TrafficAgentsPool[TrafficAgentRespawnList[i]].PositiveDeltaCount = 0;
		}
	}

}



//Calculate which cell the player is within
void ATrafficManager::UpdatePlayerCell()
{
	//Get PlayerLocation
	FVector PlayerLocation = PlayerWorldLocation;

	//Resolve Grid Position
	FVector GridLocation = (PlayerLocation + CellsOffsetSpace) / IndividualCellSize;

	int GridX = FMath::Floor(GridLocation.X) + 1;
	int GridY = FMath::Floor(GridLocation.Y) + 1;

	//Resolve Cell Index in Array
	//Index = X + (Y + GridResolution)
	int CellIndex = GridY + (GridX * GridResolution.Y);
	
	CellIndex = FMath::Clamp(CellIndex, 0, TrafficCells.Num() - 1);


	if (ActiveCellIndex != CellIndex)
	{
		ActiveCellIndex = CellIndex;
		//UE_LOG(LogTemp, Warning, TEXT("TrafficManager Grid Index, %i"), CellIndex);

		ATrafficGridObject* ActiveCell = TrafficCells[CellIndex];
		ActiveCell->DebugDrawCells = true;

		ActiveSpawnCells.Empty();		
		ActiveSpawnCells.Append(ActiveCell->NeighbourCells);

		CurrentSpawnableLanes.Empty();

		for (int i = 0; i < ActiveCell->NeighbourCells.Num(); i++)
		{

			for (int j = 0; j < ActiveCell->NeighbourCells[i]->ConnectedRoads.Num(); j++)
			{

				CurrentSpawnableLanes.Add(ActiveCell->NeighbourCells[i]->ConnectedRoads[j].ReferencedLaneSpline);

			}
		}

	}

	//Debug Draw
	//UE_LOG(LogTemp, Warning, TEXT("TrafficManager UpdatePlayerCell, %i, and, %i"), GridX, GridY);


	//UE_LOG(LogTemp, Warning, TEXT("TrafficManager UpdatePlayerCell, %s"), *PlayerLocation.ToString());

}




void ATrafficManager::DestroyNeighbourCells()
{

	if (TrafficCells.Num() > 0)
	{
		//Remove all Traffic Cells that exist in the scene
		for (int i = 0; i < TrafficCells.Num(); i++)
		{
			if (TrafficCells[i] != nullptr)
			{
				TrafficCells[i]->Destroy();
			}
		}

	}


}


void ATrafficManager::CreateTrafficGrid()
{
	
	if (TrafficCells.Num() > 0)
	{
		//Remove all Traffic Cells that exist in the scene
		for (int i = 0; i < TrafficCells.Num(); i++)
		{
			if (TrafficCells[i] != nullptr)
			{
				TrafficCells[i]->Destroy();
			}
		}

	}
	
	TrafficCells.Empty();
	CellPositions.Empty();

	//RoadPoints.Empty();
	GetAllLaneSplinePoints();

	FVector CellSize = GridWorldBounds / GridResolution;
	FVector OffsetLocation = GridWorldBounds * FVector(0.5f, 0.5f, 0.5f);
	CellsOffsetSpace = OffsetLocation;
	IndividualCellSize = CellSize;

	int CellIndex = 0;

	for (int i = 0; i < GridResolution.X; i++)
	{
		
		float XLocation = CellSize.X * i;
		float YLocation = 0.0f;

		for (int j = 0; j < GridResolution.Y; j++)
		{
			YLocation = CellSize.Y * j;

			FVector CellLocation = FVector(XLocation, YLocation,  0.0f) - FVector(OffsetLocation.X, OffsetLocation.Y,0.0f);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;

			ATrafficGridObject* NewTrafficGridObject = GetWorld()->SpawnActor<ATrafficGridObject>(ATrafficGridObject::StaticClass(), CellLocation, FRotator(0.0f), SpawnParams);

			if (NewTrafficGridObject)
			{
				NewTrafficGridObject->CellSize = CellSize * 0.5f;
				NewTrafficGridObject->SetBoxExtent(CellSize * 0.5f);
				FString CellNameID = FString::Printf(TEXT("TrafficCellGrid_%d"), CellIndex);
				//NewTrafficGridObject->Rename(*CellNameID);
				NewTrafficGridObject->SetActorLabel(CellNameID);
				NewTrafficGridObject->CellID = CellIndex;
				//NewTrafficGridObject->CollisionBounds->SetBoxExtent(CellSize);

				TrafficCells.Add(NewTrafficGridObject);
				CellPositions.Add(CellLocation);

				CellIndex = CellIndex + 1;

			}
		}
	}

	AssignLanesToCells();
	AssignNeighbourCells();

}


void ATrafficManager::AssignNeighbourCells()
{

	int CellCount = TrafficCells.Num();

	//Find NeighbourCells by Distance 
	//Use this instead of the array as we may have cells that are not completly in order as some map areas require no cell generation or different densities
	
	FVector CellSize = (GridWorldBounds / GridResolution);
	float MaxSize = FMath::Max(CellSize.X, CellSize.Y);
	float DistanceSearchTolerance = FMath::Sqrt(FMath::Square(MaxSize) + FMath::Square(MaxSize)) + 100.0f;

	for (int i = 0; i < CellCount; i++)
	{

		FVector CurrentCellLocation = TrafficCells[i]->GetActorLocation();
		TrafficCells[i]->NeighbourCells.Empty();

		TArray<ATrafficGridObject*> NeighbourCells;

		for (int j = 0; j < CellCount; j++)
		{
			if (j == i)
			{
				continue;
			}

			FVector DestinationCellLocation = TrafficCells[j]->GetActorLocation();
			float DistanceBetweenCells = FVector::Distance(CurrentCellLocation, DestinationCellLocation);

			if (DistanceBetweenCells <= DistanceSearchTolerance)
			{
				NeighbourCells.Add(TrafficCells[j]);
			}
		}

		TrafficCells[i]->NeighbourCells.Append(NeighbourCells);
	}
}


//Go Through scene and get all Lane Splines. Get their points, store thier location. Reference To LaneSpline 
void ATrafficManager::GetAllLaneSplinePoints()
{

	RoadPoints.Empty();

	TArray<AActor*> SceneActors;
	TArray<ALaneSpline*> SceneLaneSplines;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALaneSpline::StaticClass(), SceneActors);

	for (AActor* Actor : SceneActors)
	{

		ALaneSpline* FoundLane = Cast<ALaneSpline>(Actor);
		if (FoundLane)
		{

			if (FoundLane->isJunctionLane == false)
			{

				UE_LOG(LogTemp, Warning, TEXT("TrafficManager GetAllLaneSplinePoints found a single lane"));

				FLanes FoundLaneSpline;

				//Get Spline Center Point
				USplineComponent* LaneSplineComponent;
				LaneSplineComponent = FoundLane->GetComponentByClass<USplineComponent>();

				//Loop through all the SplinePoints and append to our array
				int SplinePointCount = LaneSplineComponent->GetNumberOfSplinePoints();
				int SplineSegementCount = LaneSplineComponent->GetNumberOfSplineSegments();

				TArray<FVector> PointLocation;
				TArray<int> PointID;

				FLaneSplinePoints LaneSplinePoints;

				for (int i = 0; i < SplinePointCount; i++)
				{
					PointID.Add(i);
					PointLocation.Add(LaneSplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
				}

				LaneSplinePoints.ReferencedLaneSpline = FoundLane;
				LaneSplinePoints.PointID = PointID;
				LaneSplinePoints.PointLocation = PointLocation;

				RoadPoints.Add(LaneSplinePoints);

			}
		}
	}
	


}


void ATrafficManager::AssignLanesToCells()
{
	//Go through each cell. Check its distance to our points. *This is slow, but its only done once in the editor
	//Write to each cell, the associated Road and nearest points (This is so long roads that intersect multiple cells we 
	//can spawn vehicles near their points

	int CellCount = TrafficCells.Num();
	int RoadCount = RoadPoints.Num();

	FVector CellSize = (GridWorldBounds / GridResolution) * 0.5f;
	float MaxSize = FMath::Max(CellSize.X, CellSize.Y);
	float DistanceSearchTolerance = FMath::Sqrt(FMath::Square(MaxSize) + FMath::Square(MaxSize));

	UE_LOG(LogTemp, Warning, TEXT("Traffic Manager::Square Root Result %f"), DistanceSearchTolerance);

	//For Each Cell
	for (int i = 0; i < CellCount; i++)
	{
		//Clear All Connected Lanes for this current cell
		TrafficCells[i]->ConnectedRoads.Empty();

		FVector CurrentCellLocation = TrafficCells[i]->GetActorLocation();

		//For Each Road
		for (int j = 0; j < RoadCount; j++)
		{

			FConnectedRoads ConnectedRoads;
			TArray<int> OverallapingPoint;
			
			//For Each Point within the Road
			for (int p = 0; p < RoadPoints[j].PointLocation.Num(); p++)
			{
				FVector PointLocation = RoadPoints[j].PointLocation[p];
				float PointDistance = FVector:: Distance(CurrentCellLocation, PointLocation);

				if (PointDistance <= DistanceSearchTolerance)
				{
					OverallapingPoint.Add(p);
				}
			}

			if (OverallapingPoint.Num() != 0)
			{
				ConnectedRoads.ReferencedLaneSpline = RoadPoints[j].ReferencedLaneSpline;
				ConnectedRoads.PointID = OverallapingPoint;

				TrafficCells[i]->ConnectedRoads.Add(ConnectedRoads);
			}

		}

	}

}


// Called when the game starts or when spawned
void ATrafficManager::BeginPlay()
{
	Super::BeginPlay();


	//Spawn all our initial traffic agents

	//Get Roads
	GetLanes();
	InitialiseTrafficAgents();

	//Spawn Traffic Agents


}

// Called every frame
void ATrafficManager::Tick(float DeltaTime)
{

	PlayerWorldLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();


	Super::Tick(DeltaTime);

	//Update our Player Vehicle Location and Return Current Cell
	UpdatePlayerCell();

	//Update Our Vehicle Data every n frames (every 5 seconds?)
	UpdateTrafficAgents();

	RespawnTrafficAgents();


}

