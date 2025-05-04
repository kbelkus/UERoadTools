// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseTrafficVehicle.h"
#include "Kismet/KismetMathLibrary.h"
#include "TrafficWheelComponent.h"


// Sets default values
ABaseTrafficVehicle::ABaseTrafficVehicle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>("VehicleBody");
	VehicleMesh->SetupAttachment(GetRootComponent());


	//Create Wheel Offset Vectors //Saves some multiplications
	for (int i = 0; i < WheelOffsets.Num(); i++)
	{
		FVector StartLocation = FVector(0.0f, 0.0f, 0.0f);

		//Create a vector
		FVector Direction = WheelOffsets[i] - StartLocation;
		Direction.Normalize();

		float Distance = FVector::Distance(StartLocation, WheelOffsets[i]);

		//Direction* Distance;

		WheelVectors.Add(Direction * Distance);
		WheelRaycastPositions.Add(FVector(0, 0, 0));

	}

	//V2 Fake Physics - Add Wheel Components
	for (int i = 0; i < WheelOffsets.Num(); i++)
	{
		FString ComponentName = "WheelComponent" + FString::FromInt(i);
		TObjectPtr<UTrafficWheelComponent> WheelComponent = CreateDefaultSubobject<UTrafficWheelComponent>(*ComponentName);
		
		FWheelData NewWheel;

		NewWheel.WheelComponent = WheelComponent;
		WheelComponent->SetWorldLocation(this->GetActorLocation() + WheelOffsets[i]);
		WheelComponent->SetRelativeRotation(this->GetActorRightVector().ToOrientationQuat());

		WheelData.Add(NewWheel);
	}

}

void ABaseTrafficVehicle::OnConstruction(const FTransform& Transform)
{

	Super::OnConstruction(Transform);

	//CAN REMOVE ALL THIS AFTER WE FIX PHYSICS
	for (int i = 0; i < WheelData.Num(); i++)
	{
		WheelData[i].WheelComponent->SetRelativeLocation(WheelOffsets[i]);
	}

	for (int i = 0; i < WheelData.Num(); i++)
	{
		WheelData[i].WheelComponent->SetRelativeLocation(WheelOffsets[i]);
	}

	//Temp Debug Draw Location Remove this later, trying to debug phyics bs
	for (int i = 0; i < WheelData.Num(); i++)
	{
		FVector ActorLocation = this->GetActorLocation();
		FVector WheelLocal = ActorLocation + WheelData[i].WheelComponent->GetComponentLocation();

		DrawDebugSphere(GetWorld(), GetActorLocation() + WheelData[i].WheelComponent->GetComponentLocation(), 100.0f, 16, FColor::Yellow, true, 100.0f, 1, 2.0f);
		UE_LOG(LogTemp, Log, TEXT("WheelLocation at spawn: id: %d ,  %s"), i, *WheelLocal.ToString());
		//UE_LOG(LogTemp, Log, TEXT("Actor Location: ,  %s"), i, *ActorLocation.ToString());
	}
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

	//Set our Vehicles Current Position
	PositionAlongSplineLength = SpawnPositionAlongSplineLength;


	//Set Our Static Mesh // Move to other function and update traffic manager
	//VehicleMesh.Set

}

void ABaseTrafficVehicle::VehicleTargetUpdate(float dt)
{

	//Get the current Vehicle Position
	//Add an offset
	// Check if distance exceeds tolerance
	//if end of lane, get next lane data
	//can proceed, can not set bool and next lane data
	//if can not proceed, set target status to stop
	
	const float TargetDistance = 800.0f;
	float TargetDistanceAlongSpline = PositionAlongSplineLength + TargetDistance;

	//Get Location
	TargetLocation = CurrentSpline->GetLocationAtDistanceAlongSpline(TargetDistanceAlongSpline, ESplineCoordinateSpace::World);

	if (TargetDistanceAlongSpline >= CurrentLaneLength && FoundNextLane == false)
	{
		GetNextLane();
	}

	//Pole Next Lane Status to see if signal has changed
	if (NextLane && IgnoreSignal == false)
	{
		NextLaneStatus = NextLane->LaneStatus;
	}

	//Draw Debug Colours
	FColor StatusIndicator = FColor::Blue;

	if (NextLaneStatus == 0 && IgnoreSignal == false)
	{
		StatusIndicator = FColor::Red;
	}

	DrawDebugSphere(GetWorld(), TargetLocation, 100.0f, 16, StatusIndicator, false, 1.0, 1, 2.0f);

}



//Logic for updating regular driving vehicle
void ABaseTrafficVehicle::Driving(float dt)
{
	float PredictedDistance = VehicleSpeed * dt;
	RayStartPosition = CurrentSpline->GetWorldLocationAtDistanceAlongSpline((PositionAlongSplineLength + 200.0f) + PredictedDistance) + RayHeightOffset;

	CollisionCheck();

	float DistanceToTarget = FVector::Distance(TargetLocation, RayStartPosition);

	if (IgnoreSignal == true)
	{
		NextLaneStatus = 1;
	}

	//Increment Position Along Spline
	//At the current speed, how much distance *SHOULD* we have travelled D = T*S
	
	//Solve Speed
	if (detectedObstacle == true || NextLaneStatus == 0)
	{

		//Normalized Distance
		DistanceToObstacle = FMath::Clamp(FVector::Distance(CollidedObjectLocation, RayStartPosition),0,500);

		float MinDistanceTo = FMath::Min(DistanceToTarget, DistanceToObstacle);

		float NormalizedDistance = 0.002 * (MinDistanceTo - 200.0f);

		VehicleSpeed = FMath::Abs(FMath::Lerp(NormalizedDistance, 0.0f, 800.0f));

		//Get Distance to Detected Obstacle or Target Object and compare who is smaller
		//VehicleSpeed = 0.0f;


	}
	else
	{
		VehicleSpeed = 800.0f;
	}


	float Distance = VehicleSpeed * dt;

	float NewPositionAlongSpline = PositionAlongSplineLength + Distance;
	PositionAlongSplineLength = FMath::Clamp(NewPositionAlongSpline, 0.0f, CurrentLaneLength);

	//If we hit the end of our spline get our next spline

	if (PositionAlongSplineLength >= CurrentLaneLength)
	{
		//GetNextLane();
		//Sample Next Lane from our store variables
		 
		//Reset all our Lane Info
		CurrentLane = NextLane;
		CurrentSpline = NextSpline;
		CurrentLaneLength = NextLaneLength;
		PositionAlongSplineLength = NextPositionAlongSplineLength; //Rename this var and add logic for reverse splines
		LaneDrivingDirection = NextLaneDrivingDirection;
		LaneStatus = NextLaneStatus;

		FoundNextLane = false;

		//Car has now entered new lane spline from here, tell it it can ignroe the signal
		IgnoreSignal = true;

	}
	
	//RENAME THESE TO SPLINE
	VehicleWorldLocation = CurrentSpline->GetWorldLocationAtDistanceAlongSpline(PositionAlongSplineLength);
	VehicleWorldRotation = CurrentSpline->GetWorldRotationAtDistanceAlongSpline(PositionAlongSplineLength);

	FVector ActorWorldLocation = this->GetActorLocation();


	FVector VehicleLocation = ResolveVehicleLocation(VehicleWorldLocation, VehicleWorldRotation);

	//Current Forard
	FVector MyRotation = this->GetActorForwardVector();

	FRotator RotationToBase = FRotationMatrix::MakeFromX(MyRotation).Rotator();

	//--- BASIC PHYSICS CALC MOVE TO FUNCTION
	for (int i = 0; i < WheelVectors.Num(); i++)
	{

		//Draw a Sphere Raycast Down
		FVector RelativeVector1 = RotationToBase.RotateVector(WheelVectors[i]);
		
		//Offset
		FVector RelativeLocation = VehicleWorldLocation + (RelativeVector1 * 1.0);


		//Draw a Ray Downwards
		

		//From some specific points raycast downwards and find the ground position
		FVector SplinePointLocation = (RelativeLocation + FVector(0,0, ActorWorldLocation.Z));

		FVector RayStart = SplinePointLocation + FVector(0.0f, 0.0f, 300.0f);
		FVector RayEnd = SplinePointLocation + FVector(0.0f, 0.0f, -200.0f);

		FVector ResolvedWorldLocation = RelativeLocation;


		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_WorldDynamic, CollisionParms);

		if (bHit == true)
		{
			ResolvedWorldLocation = FVector(RelativeLocation.X, RelativeLocation.Y, HitResult.Location.Z);

		}

		//This works
		//DrawDebugSphere(GetWorld(), ResolvedWorldLocation, 50.0f, 16, FColor::White, false , 0.1, 5, 1.0f);

		WheelRaycastPositions[i] = ResolvedWorldLocation;

	}

	FVector AveragedVehicleLocation = (WheelRaycastPositions[0] + WheelRaycastPositions[1] + WheelRaycastPositions[2] + WheelRaycastPositions[3]) * 0.25f;

	//for all the wheel positions resolve the vehicle height and rotation
	//First Corner
	FVector BottomLeftForward = WheelRaycastPositions[3] - WheelRaycastPositions[1];
	FVector BottomLeftRight = WheelRaycastPositions[3] - WheelRaycastPositions[2];

	//DrawDebugLine(GetWorld(), WheelRaycastPositions[2], WheelRaycastPositions[0], FColor::Red, false, 0.1f, 5, 1.0f);
	//DrawDebugLine(GetWorld(), WheelRaycastPositions[2], WheelRaycastPositions[3], FColor::Green, false, 0.1f, 5, 1.0f);

	FVector TopRightBack = WheelRaycastPositions[0] - WheelRaycastPositions[2];
	FVector TopRightLeft = WheelRaycastPositions[0] - WheelRaycastPositions[1];

	FVector FirstTriangleAngle = FVector::CrossProduct(BottomLeftForward, BottomLeftRight);
	FirstTriangleAngle.Normalize();
	
	//DrawDebugLine(GetWorld(), WheelRaycastPositions[2], WheelRaycastPositions[2] + (-FirstTriangleAngle * 500.0f), FColor::Red, false, 0.1f, 5, 1.0f);
	
	FVector SecondTriangleAngle = FVector::CrossProduct(TopRightBack, TopRightLeft);
	SecondTriangleAngle.Normalize();



	FVector AverageAngle = (FirstTriangleAngle + SecondTriangleAngle) * 0.5f;
	AverageAngle.Normalize();

	DrawDebugLine(GetWorld(), AveragedVehicleLocation, AveragedVehicleLocation + (-AverageAngle * 500.0f), FColor::Red, false, 0.1f, 5, 1.0f);

	FVector RollPitchRotationTarget = ActorWorldLocation + (AverageAngle * 800.0f);

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ActorWorldLocation, RollPitchRotationTarget);

	//FRotator NewRot = FMath::RInterpTo(StaticMesh2->GetComponentRotation(), PlayerRot, DeltaTime, 2);
	
	//Draw a Sphere to test
	//DrawDebugSphere(GetWorld(), RollPitchRotationTarget, 100.0f, 16, FColor::Cyan, true, 0.1f, 5, 2.0f);
	//DrawDebugLine(GetWorld(), ActorWorldLocation, RollPitchRotationTarget, FColor::Magenta, true, 0.1f, 5, 30.0f);

	//MERGE ROTATORS
	float yaw = VehicleWorldRotation.Yaw;
	FRotator YawRotation(0, yaw, 0);

	FMatrix GroundMatrix = FRotationMatrix::MakeFromZ(AverageAngle);
	FRotator GroundRotation = GroundMatrix.Rotator();
	FRotator CombinedRotation(LookAtRotation.Roll, YawRotation.Yaw, LookAtRotation.Pitch);

	//TEST
	const FVector UpVector = this->GetActorUpVector();
	FVector RotationAxis = FVector::CrossProduct(UpVector, AverageAngle);
	RotationAxis.Normalize();

	float RotationAngleRad = acosf(FVector::DotProduct(UpVector, AverageAngle));
	FQuat Quat = FQuat(RotationAxis, RotationAngleRad);
	FQuat NewQuat = Quat * this->GetActorQuat();
	FRotator NewRotator = NewQuat.Rotator();
	NewRotator.Yaw = YawRotation.Yaw;
	this->SetActorRelativeRotation(NewRotator);




	//Get New Position Along Spline
	//VehicleWorldLocation = CurrentSpline->GetWorldLocationAtDistanceAlongSpline(PositionAlongSplineLength);
	//VehicleWorldRotation = CurrentSpline->GetWorldRotationAtDistanceAlongSpline(PositionAlongSplineLength);

	this->SetActorLocation(AveragedVehicleLocation);
	//this->SetActorRotation(CombinedRotation);

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
		ALaneSpline* NewLaneSpline = CurrentLane->LaneConnections[RandomLaneSelection].LaneReference;

		//Reset all our Lane Info
		NextLane = NewLaneSpline;
		NextSpline = NewLaneSpline->ReturnSpline();
		NextLaneLength = NextSpline->GetSplineLength();
		NextPositionAlongSplineLength = 0.0; //Rename this var and add logic for reverse splines
		NextLaneDrivingDirection = NewLaneSpline->LaneDirection;
		NextLaneStatus = NewLaneSpline->LaneStatus;

		FoundNextLane = true;
		IgnoreSignal = false;
	}

}


//Check forward for obstacles, stop the car if objstacle detected
void ABaseTrafficVehicle::CollisionCheck()
{

	FVector EndLocation = RayStartPosition + (this->GetActorForwardVector() * 500.0f);
	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStartPosition, EndLocation, ECC_WorldStatic, CollisionParms);

	FColor Clear = FColor::Green;
	FColor Blocked = FColor::Red;
	FColor LineColour = Clear; 

	if (bHit == true)
	{
		LineColour = Blocked;		
		AActor* HitActor = HitResult.GetActor();

		if (HitActor)
		{

			CollidedObjectLocation = HitResult.Location;
			DrawDebugPoint(GetWorld(), CollidedObjectLocation, 5.0f, FColor::Red, false, 1.0f, 1);

			CollidedObject = HitActor->GetActorLabel();
			detectedObstacle = true;

			//IF TRAFFIC
			ABaseTrafficVehicle* TrafficVehicle = Cast<ABaseTrafficVehicle>(HitActor);

			if (TrafficVehicle != nullptr)
			{
				CollidedLaneDrivingDirection = TrafficVehicle->LaneDrivingDirection;
				//CollidedObject = TrafficVehicle->GetActorLabel();

				//Test for Detecting Lane
				//if (LaneDrivingDirection == CollidedLaneDrivingDirection)
				//{
				//	detectedObstacle = true;
				//}

			}

			//IF WHATEVER ELSE

		}
	}
	else
	{
		LineColour = Clear;
		detectedObstacle = false;
		DistanceToObstacle = 500.0f;
		
	}


	//Temp DrawLine
	//DrawDebugLine(GetWorld(), RayStartPosition, EndLocation, LineColour, false, 1.0f, 0.0, 1.0f);


}


//Solve the current world height offset for this vehicle + Rotation based on wheels and velocity vector
FVector ABaseTrafficVehicle::ResolveVehicleLocation(FVector SplineLocation, FRotator Rotation)
{

	//From some specific points raycast downwards and find the ground position
	FVector SplinePointLocation = SplineLocation;

	FVector RayStart = SplinePointLocation + FVector(0.0f, 0.0f, 100.0f);
	FVector RayEnd = SplinePointLocation + FVector(0.0f, 0.0f, -200.0f);
	
	FVector ResolvedWorldLocation = SplineLocation;


	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_WorldDynamic, CollisionParms);

	if (bHit == true)
	{
		ResolvedWorldLocation = FVector(SplineLocation.X, SplineLocation.Y, HitResult.Location.Z);

	}


	return ResolvedWorldLocation;

}



//Draw a Target in front the car for collision logic
void ABaseTrafficVehicle::DebugDrawTarget()
{


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

	//Temp Debug Draw Location Remove this later, trying to debug phyics bs
	for (int i = 0; i < WheelData.Num(); i++)
	{
		FVector WheelLocal = GetActorLocation() + WheelData[i].WheelComponent->GetComponentLocation();

		DrawDebugSphere(GetWorld(),GetActorLocation() + WheelData[i].WheelComponent->GetComponentLocation(), 100.0f, 16, FColor::Yellow, true, 100.0f, 1, 2.0f);
		UE_LOG(LogTemp, Log, TEXT("WheelLocation at spawn: id: %d ,  %s"), i, *WheelLocal.ToString());
		//UE_LOG(LogTemp, Log, TEXT("Actor Location: ,  %s"), i, *GetActorLocation().ToString());
	}



}

// Called every frame
void ABaseTrafficVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (UseV2Physics)
	{
		DrivingVersionTwo(DeltaTime);
	}
	else
	{

		if(isValidVehicle == false)
		{
			return;
		}


		//Get Current Spline Status for Checking
		if (CurrentLane)
		{
			LaneStatus = CurrentLane->LaneStatus;

			//if(SpawnPositionAlongSplineLength)
		}


		if (MarkedForRespawn == true)
		{
			//DrawDebugSphere(GetWorld(), this->GetActorLocation() + FVector(0, 0, 300),200.0f, 16, FColor::Red, false, 0.1, 10, 10);

		}

		//Vehicle Logic
		//Assume its always driving for now
		VehicleTargetUpdate(DeltaTime);


		Driving(DeltaTime);
	}
}

void ABaseTrafficVehicle::DrivingVersionTwo(float DeltaTime)
{
	FlushPersistentDebugLines(GetWorld());

	FTransform CarCurrentTransform = this->GetActorTransform();

	FVector AverageWheelsPosition;
	float TotalSuspensionForce = 0.f;

	for (int i = 0; i < WheelData.Num(); i++)
	{
		FTransform CombinedTransforms = WheelData[i].WheelComponent->GetComponentTransform() * this->GetActorTransform();
		FMatrix TransformM;

		//Wheel Center Marker
		//DrawDebugSphere(GetWorld(), CombinedTransforms.GetLocation(), 50.0f, 16, FColor::Purple, true, 100.0f, 1, 2.0f);
		
		TransformM = CombinedTransforms.ToMatrixWithScale();

		//Cast Down Form Wheel
		FVector RayStart = CombinedTransforms.GetLocation() + (FVector::UpVector * 20.0f); //STart Point
		FVector RayEnd = RayStart - FVector::UpVector * WheelData[i].MaxLength; //End Point

		//Debug Draw
		DrawDebugSphere(GetWorld(), RayStart, 10.0f, 16, FColor::Green, true, 1.0f, 8, 2.0f);


		UE_LOG(LogTemp, Log, TEXT("WheelLocation id: %d ,  %s"),i, *RayStart.ToString());

		FVector ResolvedWorldLocation = CombinedTransforms.GetLocation();

		//DrawDebugLine(GetWorld(), RayStart, RayEnd, FColor::Blue, true, 1.0f, 2, 1.0f);

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_WorldDynamic, CollisionParms);

		if (bHit == true)
		{

			UE_LOG(LogTemp, Log, TEXT("RayHit"));

			float HitDistance = (RayStart - HitResult.Location).Size();
			UE_LOG(LogTemp, Log, TEXT("Hit Distance: %f"), HitDistance);
			WheelData[i].CurrentCompression = FMath::Clamp(WheelData[i].RestLength - HitDistance, 0.0f, WheelData[i].RestLength);

			DrawDebugSphere(GetWorld(), RayStart + FVector(0,0,WheelData[i].CurrentCompression), 50.0f, 16, FColor::Purple, true, 1.0f, 2, 2.0f);

			WheelData[i].Velocity = (WheelData[i].CurrentCompression - WheelData[i].PreviousCompression) / DeltaTime;
			WheelData[i].PreviousCompression = WheelData[i].CurrentCompression; //

			float SpringForce = WheelData[i].CurrentCompression * WheelData[i].SuspensionStiffness;
			float DampingForce = WheelData[i].Velocity * WheelData[i].SuspensionDamping;

			float NetForce = SpringForce - DampingForce;

			TotalSuspensionForce += NetForce;

			ResolvedWorldLocation = FVector(ResolvedWorldLocation.X, ResolvedWorldLocation.Y, WheelData[i].CurrentCompression);

			FVector LocalOffset = FVector(0, 0, WheelData[i].CurrentCompression);

			WheelData[i].WheelComponent->SetWorldLocation(WheelData[i].WheelComponent->GetComponentLocation() + LocalOffset);
			CombinedTransforms.SetLocation(WheelData[i].WheelComponent->GetComponentLocation() + LocalOffset);

			DrawDebugLine(GetWorld(), RayStart, HitResult.Location, FColor::Green, true, 1.0f, 2, 2.0f);


		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Ray Miss"));

			WheelData[i].CurrentCompression = 0.0f;
			WheelData[i].WheelComponent->SetRelativeLocation(WheelOffsets[i]);
			//CombinedTransforms.SetLocation(WheelData[i].WheelComponent->GetComponentLocation() + LocalOffset);
			
			DrawDebugLine(GetWorld(), CombinedTransforms.GetLocation(), RayEnd, FColor::Red, false, 0.1f, 2, 2.0f);
		}


		//Body Physics
		float Weight = CarMass * Gravity;
		float ForceDelta = TotalSuspensionForce - Weight;

		float Acceleration = ForceDelta / CarMass;
		VerticalVelocity += Acceleration * DeltaTime;
		CarBodyHeightOffset += VerticalVelocity * DeltaTime;

		//ClampBounce

		VerticalVelocity *= 0.99f; //Damping

		FVector BaseLocation = GetActorLocation(); //<--- Get From Spline
		FVector TempLocal = BaseLocation + (FVector::UpVector * CarBodyHeightOffset);

		CombinedTransforms.SetLocation(WheelData[i].WheelComponent->GetComponentLocation());

		TransformM = CombinedTransforms.ToMatrixWithScale();

		//SetActorLocation(TempLocal);
				
		DrawDebugCircle(GetWorld(), TransformM, 100.0f, 16, FColor::Yellow, true, 1.0f, 2, 2.0f, true);

		DrawDebugBox(GetWorld(), TempLocal, FVector(300, 200, 180), FColor::Orange, true, 1.0f, 3, 2.0f);

		AverageWheelsPosition += ResolvedWorldLocation;

	}

	//Averaged Wheel Position
	



	//Fake Set Car Position
	//Apply Gravity
	DrawDebugSphere(GetWorld(), AverageWheelsPosition * 0.25f, 100.0f, 16, FColor::Red, false, 1.0, 1, 2.0f);
	



}

