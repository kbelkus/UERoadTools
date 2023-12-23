// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseTrafficVehicle.h"

// Sets default values
ABaseTrafficVehicle::ABaseTrafficVehicle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>("VehicleBody");
	VehicleMesh->SetupAttachment(GetRootComponent());
}

void ABaseTrafficVehicle::SetSpawnLocation()
{

	//Get Spline Location and Init spawn location

	if (CurrentLane == nullptr)
	{
		return;
	}

	CurrentSpline = CurrentLane->ReturnSpline();

	if (CurrentSpline)
	{
		VehicleWorldLocation = CurrentSpline->GetWorldLocationAtDistanceAlongSpline(SpawnPositionAlongSplineLength);
		VehicleWorldRotation = CurrentSpline->GetWorldRotationAtDistanceAlongSpline(SpawnPositionAlongSplineLength);
		
		this->SetActorLocation(VehicleWorldLocation);
		this->SetActorRotation(VehicleWorldRotation);

		CurrentLaneLength = CurrentSpline->GetSplineLength();

		LaneDrivingDirection = CurrentLane->LaneDirection;

		isValidVehicle = true;
	}

}


//Logic for updating regular driving vehicle
void ABaseTrafficVehicle::Driving(float dt)
{
	float PredictedDistance = VehicleSpeed * dt;
	RayStartPosition = CurrentSpline->GetWorldLocationAtDistanceAlongSpline((SpawnPositionAlongSplineLength + 100.0f) + PredictedDistance) + RayHeightOffset;

	CollisionCheck();

	//Increment Position Along Spline
	//At the current speed, how much distance *SHOULD* we have travelled D = T*S

	//Solve Speed
	if (detectedObstacle == true || LaneStatus == 0)
	{
		VehicleSpeed = 0.0f;
	}
	else
	{
		VehicleSpeed = 800.0f;
	}



	float Distance = VehicleSpeed * dt;

	float NewPositionAlongSpline = SpawnPositionAlongSplineLength + Distance;
	SpawnPositionAlongSplineLength = FMath::Clamp(NewPositionAlongSpline, 0.0f, CurrentLaneLength);

	//If we hit the end of our spline get our next spline
	if (SpawnPositionAlongSplineLength >= CurrentLaneLength)
	{
		GetNextLane();
	}


	//Get New Position Along Spline
	VehicleWorldLocation = CurrentSpline->GetWorldLocationAtDistanceAlongSpline(SpawnPositionAlongSplineLength);
	VehicleWorldRotation = CurrentSpline->GetWorldRotationAtDistanceAlongSpline(SpawnPositionAlongSplineLength);

	this->SetActorLocation(VehicleWorldLocation);
	this->SetActorRotation(VehicleWorldRotation);

}


void ABaseTrafficVehicle::GetNextLane()
{

	//Get our current splines connected splines
	//TArray<ALaneSpline> ConnectedLanes;
	int ConnectedLaneCount = CurrentLane->LaneConnections.Num();

	if (ConnectedLaneCount != 0)
	{

		//Randomly Select Lane Logic Here - or pathfinding later
		int RandomLaneSelection = GetRandomLaneID(0, ConnectedLaneCount - 1);

		UE_LOG(LogTemp, Warning, TEXT("Vehicle Random ID, %i"), RandomLaneSelection);

		ALaneSpline* NewLaneSpline = CurrentLane->LaneConnections[RandomLaneSelection].LaneReference;

		//Reset all our Lane Info
		CurrentLane = NewLaneSpline;
		CurrentSpline = NewLaneSpline->ReturnSpline();
		CurrentLaneLength = CurrentSpline->GetSplineLength();
		SpawnPositionAlongSplineLength = 0.0; //Rename this var and add logic for reverse splines
		LaneDrivingDirection = NewLaneSpline->LaneDirection;
		LaneStatus = NewLaneSpline->LaneStatus;
	}

}


//Check forward for obstacles, stop the car if objstacle detected
void ABaseTrafficVehicle::CollisionCheck()
{

	FVector EndLocation = RayStartPosition + (this->GetActorForwardVector() * 500.0f);

	FHitResult HitResult;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStartPosition, EndLocation, ECC_WorldDynamic, CollisionParms);

	FColor Clear = FColor::Green;
	FColor Blocked = FColor::Red;

	FColor LineColour = Clear; 

	if (bHit == true)
	{
		LineColour = Blocked;
		
		AActor* HitActor = HitResult.GetActor();

		if (HitActor)
		{

			ABaseTrafficVehicle* TrafficVehicle = Cast<ABaseTrafficVehicle>(HitActor);

			if (TrafficVehicle != nullptr)
			{
				CollidedLaneDrivingDirection = TrafficVehicle->LaneDrivingDirection;

				if (LaneDrivingDirection == CollidedLaneDrivingDirection)
				{
					detectedObstacle = true;
				}

			}
		}

	}
	else
	{
		LineColour = Clear;
		detectedObstacle = false;
	}







	//Temp DrawLine
	DrawDebugLine(GetWorld(), RayStartPosition, EndLocation, LineColour, false, 1.0f, 0.0, 1.0f);


}


// Called when the game starts or when spawned
void ABaseTrafficVehicle::BeginPlay()
{
	Super::BeginPlay();


	//Random ID TEst
	//int RandomID = FMath::RandInit(GetUniqueID());

	//FRandomStream Stream;
	//Stream.Initialize(GetUniqueID());
	CollisionParms.AddIgnoredActor(this);
	
	SetSpawnLocation();
}

// Called every frame
void ABaseTrafficVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(isValidVehicle == false)
	{
		return;
	}


	//Vehicle Logic
	//Assume its always driving for now
	Driving(DeltaTime);

}

