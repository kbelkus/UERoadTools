// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseTrafficVehicle.h"
#include "Kismet/KismetMathLibrary.h"
#include "TrafficWheelComponent.h"
#include "Engine/DataAsset.h"
#include "AIVehicleDynamicsUtilities.h"


// Sets default values
ABaseTrafficVehicle::ABaseTrafficVehicle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent->SetMobility(EComponentMobility::Movable);
	SetRootComponent(RootComponent);

	VehicleBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>("VehicleBody");
	VehicleBodyMesh->SetupAttachment(GetRootComponent());


	//Create Wheel Offset Vectors //Saves some multiplications //CAN MAYBE DEP THIS
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

}

void ABaseTrafficVehicle::OnConstruction(const FTransform& Transform)
{

	Super::OnConstruction(Transform);

	CalculateCOM();
}

bool ABaseTrafficVehicle::BuildVehicleData()
{
	bool VehicleIsBuilt = false;

	//Load DataAsset - if not load default on disk
	const UVehicleDynamicsDataAsset* VehicleDynamics = VehicleDynamicsDataAsset.Get();

	if (!VehicleDynamics)
	{
		VehicleDynamics = LoadObject<UVehicleDynamicsDataAsset>(nullptr, TEXT("/Game/Data/VehicleData/GenericCar.GenericCar"));
	}

	if (VehicleDynamics)
	{
		VehicleDynamicsStruct = VehicleDynamics->VehicleDynamics;
		VehicleIsBuilt = true;
	}

	//Spawn Wheels
	CreateWheelMeshes();

	//Precache some vehicle data
	PreCalculatedWheelCircumference = 2 * 3.14159 * VehicleDynamicsStruct.WheelRadius;

	return VehicleIsBuilt;
}

void ABaseTrafficVehicle::CreateWheelMeshes()
{
	
	for (int i = 0; i < VehicleDynamicsStruct.WheelOffsets.Num(); i++)
	{
		FString ComponentName = TEXT("WheelMesh") + i;

		TObjectPtr<UStaticMeshComponent> WheelMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), NAME_None, RF_Transactional);
		WheelMeshComponent->SetupAttachment(GetRootComponent());
		WheelMeshComponent->RegisterComponent();
		this->AddInstanceComponent(WheelMeshComponent);

		WheelMeshComponent->SetStaticMesh(VehicleDynamicsStruct.WheelMeshes.LoadSynchronous());

		WheelMeshes.Add(WheelMeshComponent);

		WheelMeshComponent->SetWorldScale3D(VehicleDynamicsStruct.WheelScales[i]);
	}

	VehicleBodyMesh->SetStaticMesh(VehicleDynamicsStruct.BodyMesh.LoadSynchronous());

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

	DrawDebugSphere(GetWorld(), TargetLocation, 20.0f, 16, StatusIndicator, false, 1.0, 1, 1.0f);

}

//Logic for updating regular driving vehicle
void ABaseTrafficVehicle::Driving(float dt)
{
	float PredictedDistance = VehicleSpeed * dt;
	RayStartPosition = CurrentSpline->GetWorldLocationAtDistanceAlongSpline((PositionAlongSplineLength + 200.0f) + PredictedDistance) + (RayHeightOffset + FVector(0,0,GetActorLocation().Z));

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

		VehicleSpeed = FMath::Abs(FMath::Lerp(NormalizedDistance, 0.0f, VehicleMaxSpeed));

		//Get Distance to Detected Obstacle or Target Object and compare who is smaller
		//VehicleSpeed = 0.0f;
	}
	else
	{
		VehicleSpeed = VehicleMaxSpeed;
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

		//Car has now entered new lane spline from here, tell it it can ignore the signal
		IgnoreSignal = true;

	}
	
	//RENAME THESE TO SPLINE
	VehicleWorldLocation = CurrentSpline->GetWorldLocationAtDistanceAlongSpline(PositionAlongSplineLength);
	VehicleWorldRotation = CurrentSpline->GetWorldRotationAtDistanceAlongSpline(PositionAlongSplineLength);

	FVector ActorWorldLocation = this->GetActorLocation();

	FTransform SplineTransform;
	SplineTransform.SetLocation(VehicleWorldLocation);
	SplineTransform.SetRotation(VehicleWorldRotation.Quaternion());

	ResolveVehiclePhysics(dt, VehicleWorldLocation, VehicleWorldRotation);


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

	DrawDebugLine(GetWorld(), RayStartPosition, EndLocation, FColor::Red, true, -1.0f, 2, 2.0f);

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

//Solve the current world height offset for this vehicle + Rotation based on wheels and velocity vector DEP THIS
FVector ABaseTrafficVehicle::ResolveVehicleLocation(FVector SplineLocation, FRotator Rotation)
{

	//From some specific points raycast downwards and find the ground position
	FVector SplinePointLocation = SplineLocation;

	FVector RayStart = SplinePointLocation + FVector(0.0f, 0.0f, 100.0f);
	FVector RayEnd = SplinePointLocation + FVector(0.0f, 0.0f, -200.0f);
	
	FVector ResolvedWorldLocation = SplineLocation;


	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_WorldDynamic, CollisionParms);

	//if (bHit == true)
	//{
	//	ResolvedWorldLocation = FVector(SplineLocation.X, SplineLocation.Y, HitResult.Location.Z);

	//}

	return ResolvedWorldLocation;

}

//Draw a Target in front the car for collision logic
void ABaseTrafficVehicle::DebugDrawTarget()
{


}

//Solve Suspension, WheelPositions, and Chassis Positions
void ABaseTrafficVehicle::ResolveVehiclePhysics(float InDeltaTime, FVector InSplineLocation, FRotator InSplineRotation)
{
	FTransform CurrentActorTransform = this->GetActorTransform();

	VehicleVelocity += Gravity * InDeltaTime;
	InertialTensor = VehicleDynamicsStruct.InertialTensor * VehicleDynamicsStruct.IntertialTensorScale;
	FVector TransformedCOM = CurrentActorTransform.TransformPosition(VehicleCOM);

	AccumilatedForce = FVector::ZeroVector;
	AccumilatedTorque = FVector::ZeroVector;

	FCollisionQueryParams CollisionParms;

	//For Each Wheel
	for (int i = 0; i < WheelOffsets.Num(); i++)
	{
		const FVector SupsensionAnchor = CurrentActorTransform.TransformPosition(VehicleDynamicsStruct.WheelOffsets[i] + FVector(0, 0, VehicleDynamicsStruct.SuspensionOffset));
		const FVector SuspensionTarget = SupsensionAnchor - ((FVector::UpVector * VehicleDynamicsStruct.MaxSuspensionLength) + FVector(0, 0, 50.0f)); //Change this 50cm

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, SupsensionAnchor, SuspensionTarget, ECC_WorldStatic, CollisionParms);

		if (bHit)
		{
			FVector HitLocation = HitResult.Location + FVector(0, 0, VehicleDynamicsStruct.WheelRadius);
			FVector SuspensionDirection = (SupsensionAnchor - HitLocation).GetSafeNormal();
			float SuspensionCurrentLength = FMath::Clamp((SupsensionAnchor - HitLocation).Size(),1.0f, VehicleDynamicsStruct.MaxSuspensionLength);

			 //Test Code
			float Compression = VehicleDynamicsStruct.SuspensionRestLength - SuspensionCurrentLength;

			float VelocityAlongSpring = (SuspensionPrevDistance[i] - SuspensionCurrentLength) / InDeltaTime;

			SuspensionPrevDistance[i] = SuspensionCurrentLength;

			 FVector SpringForce = VehicleDynamicsStruct.SuspensionSpringStiffness * Compression * SuspensionDirection;
			 FVector DampingForce = VehicleDynamicsStruct.SuspensionDampingCoefficient * VelocityAlongSpring * SuspensionDirection;

			FVector TotalForce = SpringForce + DampingForce;

			if (FMath::Abs(VelocityAlongSpring) < 1.0f)
			{
				VelocityAlongSpring = 0.0f;
			}
			
			AccumilatedForce += TotalForce;

			//Update WheelMeshes
			WheelMeshes[i]->SetWorldLocation(HitResult.Location + FVector(0, 0, VehicleDynamicsStruct.WheelRadius));
			
			//Handle Torque
			FVector Force = TotalForce;
			FVector ToWheel = SupsensionAnchor - (TransformedCOM + FVector(0.0f, 0.0f, VehicleDynamicsStruct.SuspensionOffset));
			FVector Torque = FVector::CrossProduct(ToWheel, Force);

			AccumilatedTorque += Torque;

			UpdateWheelRotation(i,InDeltaTime, CurrentActorTransform.GetLocation());

			//Debug Draw Outputs
			//Draw COM
			DrawDebugSphere(GetWorld(), TransformedCOM + FVector(0, 0, SuspensionOffset), 10.0f, 16, FColor::Blue, true, 1.0, 1, 0.5f);
			DrawDebugLine(GetWorld(), TransformedCOM + FVector(0, 0, SuspensionOffset), (TransformedCOM + FVector(0, 0, SuspensionOffset)) + (ToWheel), FColor::Orange, true, -1.0f, 2, 1.0f);
			//DrawDebugLine(GetWorld(), SupsensionAnchor, SupsensionAnchor + (Force * 0.0001), FColor::Turquoise, true, -1.0f, 2, 1.0f);
			//DrawDebugLine(GetWorld(), SupsensionAnchor, SupsensionAnchor + (Torque * 0.000001), FColor::Cyan, true, -1.0f, 2, 1.0f);
			DrawDebugSphere(GetWorld(), SupsensionAnchor, 10.0f, 16, FColor::Blue, true, 1.0, 1, 0.5f);
			DrawDebugSphere(GetWorld(), HitResult.Location, 10.0f, 16, FColor::Green, true, 1.0, 1, 0.5f);
			DrawDebugSphere(GetWorld(), SuspensionTarget, 25.0f, 16, FColor::Purple, true, 1.0, 1, 1.0f);
			DrawDebugLine(GetWorld(), SupsensionAnchor, HitResult.Location, FColor::Green, true, -1.0f, 1, 2.0f);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("NO HIT NO HIT"));

			//We need a better fail state here, otherwise vehicle will fall through the ground, maybe store previous frame force and just re-use it
			//VehicleVelocity = FVector(0.0f,0.0f,0.0f);
			//WheelMeshes[i]->SetWorldLocation(Anchor - (FVector::UpVector * MaxSuspesionLength));

			DrawDebugSphere(GetWorld(), SupsensionAnchor, 50.0f, 16, FColor::Blue, true, 1.0, 1, 2.0f);
			DrawDebugSphere(GetWorld(), SupsensionAnchor - (FVector::UpVector * MaxSuspensionLength), 50.0f, 16, FColor::Red, true, 1.0, 1, 1.0f);

			DrawDebugLine(GetWorld(), SupsensionAnchor, SupsensionAnchor - (FVector::UpVector * MaxSuspensionLength), FColor::Green, true, -1.0f, 1, 2.0f);

		}
	}

	VehicleVelocity += (AccumilatedForce / VehicleMass) * InDeltaTime;
	VehicleAngularVelocity += ((AccumilatedTorque * 0.0001f) / InertialTensor) * InDeltaTime;
	VehicleAngularVelocity *= 0.98f;

	if (VehicleAngularVelocity.Size() < 0.01f)
	{
		VehicleAngularVelocity = FVector::ZeroVector;
	}

	//Combine Rotatation and Transforms here
	FRotator PhysicsRotation = GetActorRotation();
	FRotator SplineRotation = InSplineRotation;

	FRotator CombinedRotation;
	CombinedRotation.Pitch = PhysicsRotation.Pitch;
	CombinedRotation.Roll = PhysicsRotation.Roll;
	CombinedRotation.Yaw = SplineRotation.Yaw;

	FQuat DeltaRotation = FQuat(VehicleAngularVelocity.GetSafeNormal(), VehicleAngularVelocity.Size() * InDeltaTime);
	ActorRotation = (DeltaRotation * CombinedRotation.Quaternion()).GetNormalized();
	this->SetActorRotation(ActorRotation);

	FVector CombinedLocation = FVector(InSplineLocation.X, InSplineLocation.Y, (this->GetActorLocation() += VehicleVelocity * InDeltaTime).Z);

	this->SetActorLocation(CombinedLocation);


	//BodyMesh->SetWorldLocation(CombinedLocation);

	DrawDebugBox(GetWorld(), (CombinedLocation + VehicleDynamicsStruct.VehicleExtendsOffset), VehicleDynamicsStruct.VehicleExtends, ActorRotation, FColor::Blue, true, 1.0f, 1, 1.0f);

	//Update Speed
	CurrentSpeed = (CombinedLocation - PreviousLocation).Size();
	PreviousLocation = CombinedLocation;

}

FVector ABaseTrafficVehicle::CalculateCOM()
{
	FVector CenterOfMass = FVector::ZeroVector;

	for (int i = 0; i < VehicleDynamicsStruct.WheelOffsets.Num(); i++)
	{
		CenterOfMass += VehicleDynamicsStruct.WheelOffsets[i];
		SuspensionPrevDistance[i] = VehicleDynamicsStruct.MaxSuspensionLength;
	}

	CenterOfMass = CenterOfMass * 0.25;
	return CenterOfMass;
}

//Update our wheel rotations for cheap animation. We can probably optimise this and do the wheel roll rotation once per frame then use it for all wheels.
void ABaseTrafficVehicle::UpdateWheelRotation(int InWheelID, float InDeltaTime, FVector InVehicleLocation)
{
	WheelRotationDelta += (CurrentSpeed / (PreCalculatedWheelCircumference * 0.01f)) * 360.0f * InDeltaTime; // degrees

	SteerAngle = FVector::DotProduct(this->GetActorRightVector(), (FVector(InVehicleLocation.X, InVehicleLocation.Y, 0.0f) - FVector(TargetLocation.X, TargetLocation.Y, 0.0f)).GetSafeNormal());
	SteerAngle = SteerAngle * VehicleDynamicsStruct.WheelMask[InWheelID];

	//DrawDebugLine(GetWorld(), InVehicleLocation, TargetLocation, FColor::Emerald, false, 1.0f, 1, 2.0f);
	//DrawDebugLine(GetWorld(), InVehicleLocation, InVehicleLocation + (this->GetActorRightVector() * 500.0f), FColor::Red, false, 1.0f, 1, 2.0f);

	FRotator SteerRotation = FRotator(0.0f, FMath::RadiansToDegrees(-SteerAngle), 0.0f);
	FRotator SpinRotation = FRotator(-WheelRotationDelta, 0.0f, 0.0f);
	FQuat FinalQuat = FQuat(SteerRotation) * FQuat(SpinRotation);

	WheelMeshes[InWheelID]->SetRelativeRotation(FinalQuat);

}


// Called when the game starts or when spawned
void ABaseTrafficVehicle::BeginPlay()
{
	Super::BeginPlay();

	bool VehicleDataIsValid = BuildVehicleData();

	if (VehicleDataIsValid)
	{
		CollisionParms.AddIgnoredActor(this);
		SetSpawnLocation();
		VehicleCOM = CalculateCOM();
		isValidVehicle = true;
	}
	else
	{
		isValidVehicle = false;
	}

}

// Called every frame
void ABaseTrafficVehicle::Tick(float DeltaTime)
{
	FlushPersistentDebugLines(GetWorld());

	Super::Tick(DeltaTime);


	UE_LOG(LogTemp, Log, TEXT("Vehicle COM %s"), *VehicleCOM.ToString());

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

//REMOVE THIS No longer needed
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
		//float Weight = CarMass * Gravity;
		//float ForceDelta = TotalSuspensionForce - Weight;

		//float Acceleration = ForceDelta / CarMass;
		//VerticalVelocity += Acceleration * DeltaTime;
		//CarBodyHeightOffset += VerticalVelocity * DeltaTime;

		////ClampBounce

		//VerticalVelocity *= 0.99f; //Damping

		//FVector BaseLocation = GetActorLocation(); //<--- Get From Spline
		//FVector TempLocal = BaseLocation + (FVector::UpVector * CarBodyHeightOffset);

		//CombinedTransforms.SetLocation(WheelData[i].WheelComponent->GetComponentLocation());

		//TransformM = CombinedTransforms.ToMatrixWithScale();

		////SetActorLocation(TempLocal);
		//		
		//DrawDebugCircle(GetWorld(), TransformM, 100.0f, 16, FColor::Yellow, true, 1.0f, 2, 2.0f, true);

		//DrawDebugBox(GetWorld(), TempLocal, FVector(300, 200, 180), FColor::Orange, true, 1.0f, 3, 2.0f);

		//AverageWheelsPosition += ResolvedWorldLocation;

	}

	//Averaged Wheel Position
	



	//Fake Set Car Position
	//Apply Gravity
	DrawDebugSphere(GetWorld(), AverageWheelsPosition * 0.25f, 100.0f, 16, FColor::Red, false, 1.0, 1, 2.0f);
	



}

