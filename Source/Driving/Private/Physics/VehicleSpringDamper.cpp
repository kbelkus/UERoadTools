// Fill out your copyright notice in the Description page of Project Settings.

#include "Physics/VehicleSpringDamper.h"
#include "Components/SplineComponent.h"
#include "LaneSpline.h"

// Sets default values
AVehicleSpringDamper::AVehicleSpringDamper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent->SetMobility(EComponentMobility::Movable);
	SetRootComponent(RootComponent);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>("BodyMesh");
	BodyMesh->SetMobility(EComponentMobility::Movable);
	BodyMesh->SetupAttachment(RootComponent);

	//Add Wheel Comps
	for (int i = 0; i < 4; i++)
	{
		FString ComponentName = "WheelMesh" + FString::FromInt(i);

		TObjectPtr<UStaticMeshComponent>  WheelMesh = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
		WheelMesh->SetMobility(EComponentMobility::Movable);
		WheelMesh->SetupAttachment(RootComponent);

		WheelMeshes.Add(WheelMesh);
	}
}

void AVehicleSpringDamper::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (InputBodyMesh && WheelMesh)
	{
		BodyMesh->SetStaticMesh(InputBodyMesh);

		for (int i = 0; i < WheelMeshes.Num(); i++)
		{
			WheelMeshes[i]->SetStaticMesh(WheelMesh);

			DrawDebugSphere(GetWorld(), this->GetActorLocation() + WheelOffset[i], 10.0f, 8, FColor::Blue, true, 1.0f, 1, 1.0f);

		}
	}

}

FTransform AVehicleSpringDamper::GetPositionAlongSpline(float InSplineDelta, FTransform& OutTargetTransform)
{

	FTransform OutTransform;
	FTransform TargetTransform;


	OutTransform.SetLocation(ActiveSplineComponent->GetLocationAtDistanceAlongSpline(InSplineDelta, ESplineCoordinateSpace::World));
	OutTransform.SetRotation((ActiveSplineComponent->GetRotationAtDistanceAlongSpline(InSplineDelta, ESplineCoordinateSpace::World)).Quaternion());

	TargetTransform.SetLocation(ActiveSplineComponent->GetLocationAtDistanceAlongSpline(InSplineDelta + 800, ESplineCoordinateSpace::World));
	TargetTransform.SetRotation((ActiveSplineComponent->GetRotationAtDistanceAlongSpline(InSplineDelta + 800, ESplineCoordinateSpace::World)).Quaternion());

	OutTargetTransform = TargetTransform;

	return OutTransform;
}

//Could move this into the main loop during refactor?
void AVehicleSpringDamper::UpdateWheelRotations(float InSpeedDelta, FTransform InSteeringTarget, float DeltaTime, FVector InVehicleForwardDirection, FVector InVehicleLocation)
{
	//float RotationDelta = (WheelCircumference / InSpeedDelta) * DeltaTime;
	RotationDelta += (InSpeedDelta / (WheelCircumference * 0.01f)) * 360.0f * DeltaTime; // degrees

	InVehicleLocation.Z = 0.0f;
	FVector SteeringTargetLocation = InSteeringTarget.GetLocation();
	SteeringTargetLocation.Z = 0.0f;

	//Steering Angle
	SteerAngle = FVector::DotProduct(InVehicleForwardDirection, (InVehicleLocation - SteeringTargetLocation).GetSafeNormal());

	

	DrawDebugLine(GetWorld(), InVehicleLocation, SteeringTargetLocation, FColor::Emerald, false, 1.0f, 1, 2.0f);
	DrawDebugLine(GetWorld(), InVehicleLocation, InVehicleLocation + (InVehicleForwardDirection * 500.0f), FColor::Red, false, 1.0f, 1, 2.0f);

	//UE_LOG(LogTemp, Log, TEXT("Angle: %f, RotationDelta %f"), SteerAngle, RotationDelta);
	//UE_LOG(LogTemp, Log, TEXT("InForwardVec: %s, RotationDelta %f"), FMath::RadiansToDegrees(SteerAngle), RotationDelta);

	for (int i = 0; i < WheelMeshes.Num(); i++)
	{

		SteerAngle = SteerAngle * WheelIDs[i];

		FRotator SteerRotation = FRotator(0.0f, FMath::RadiansToDegrees(-SteerAngle), 0.0f);
		FRotator SpinRotation = FRotator(-RotationDelta, 0.0f, 0.0f);

		FQuat FinalQuat = FQuat(SteerRotation) * FQuat(SpinRotation);
		WheelMeshes[i]->SetRelativeRotation(FinalQuat);
	}
}


// Called when the game starts or when spawned
void AVehicleSpringDamper::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < 4; i++)
	{
		OldDistance[i] = MaxSuspesionLength;
		CalculatedCOM += WheelOffset[i];
	}

	CarBodyTransform = this->GetActorTransform();

	CalculatedCOM *= 0.25;

	if (bUseSpline == true && OptionalLaneSpline != nullptr)
	{
		ActiveSplineComponent = OptionalLaneSpline->ReturnSpline();
	}
	else
	{
		bUseSpline = false;
	}

	WheelCircumference = 2 * 3.14159 * (WheelRadius * 2.0f);


}

// Called every frame
void AVehicleSpringDamper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FlushPersistentDebugLines(GetWorld());

	VehicleVelocity += FVector(0, 0, -980.0f) * DeltaTime;
	FVector AccumilatedForce = FVector::ZeroVector;
	FVector AccumulilatedTorque = FVector::ZeroVector;
	InertialTensor *= IntertialTensorScale; // tweak manuall

	FTransform TargetTransform;

	if (bUseSpline)
	{
		//Set Init Transform From Spline
		SplineDelta = SplineDelta + (DeltaTime * VehicleAcc);
		SplineTransform = GetPositionAlongSpline(SplineDelta, TargetTransform);
	}

	FVector TransformedCOM = GetActorTransform().TransformPosition(CalculatedCOM);

	for (int i = 0; i < 4; i++)
	{
		
		//RayCast for Each Wheel
		FCollisionQueryParams CollisionParms;

		FVector Anchor = GetActorTransform().TransformPosition(WheelOffset[i] + FVector(0, 0, SuspensionOffset));//(this->GetActorLocation() + FVector(0,0, SuspensionOffset)) + WheelOffset[i];
		FVector RayEnd = Anchor - ((FVector::UpVector * MaxSuspesionLength) + FVector(0,0,50.0f));

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Anchor, RayEnd, ECC_WorldStatic, CollisionParms);

		if (bHit)
		{
			FVector HitLocation = HitResult.Location + FVector(0, 0, WheelRadius);

			FVector SuspensionDirection = (Anchor - HitLocation).GetSafeNormal();

			float CurrentLength = (Anchor - HitLocation).Size();

			CurrentLength = FMath::Clamp(CurrentLength, 1, MaxSuspesionLength);

			float Compression = RestLength - CurrentLength;

			float VelocityAlongSpring = (OldDistance[i] - CurrentLength) / DeltaTime;

			//UE_LOG(LogTemp, Log, TEXT("SpringDamper Velocity Along Spring %f"), VelocityAlongSpring);

			//UE_LOG(LogTemp, Log, TEXT("Compression: %f, VelocityAlongSpring: %f"), Compression, VelocityAlongSpring);

			//UE_LOG(LogTemp, Log, TEXT("Velocity Along Spring %f"), VelocityAlongSpring);
			OldDistance[i] = CurrentLength;

			FVector SpringForce = SpringStiffness * Compression * SuspensionDirection;
			FVector DampingForce = DampingCoefficient * VelocityAlongSpring * SuspensionDirection;

			FVector TotalForce = SpringForce + DampingForce;
			
			if (FMath::Abs(VelocityAlongSpring) < 1.0f)
			{
				VelocityAlongSpring = 0.0f;
			}

			AccumilatedForce += TotalForce;

			WheelMeshes[i]->SetWorldLocation(HitResult.Location + FVector(0, 0, WheelRadius));

			//Torque?
			FVector Force = TotalForce;

			FVector ToWheel = Anchor - (TransformedCOM + FVector(0, 0, SuspensionOffset)); //Store this COM somewhere?

			
			//Draw COM
			DrawDebugSphere(GetWorld(), TransformedCOM + FVector(0, 0, SuspensionOffset), 10.0f, 16, FColor::Blue, true, 1.0, 1, 0.5f);

			//Draw To Wheel
			DrawDebugLine(GetWorld(), TransformedCOM + FVector(0, 0, SuspensionOffset), (TransformedCOM + FVector(0, 0, SuspensionOffset)) + (ToWheel), FColor::Orange, true, -1.0f, 2, 1.0f);

			FVector Torque = FVector::CrossProduct(ToWheel, Force);

			DrawDebugLine(GetWorld(), Anchor, Anchor + (Force * 0.01), FColor::Turquoise, true, -1.0f, 10.0f, 2.0f);
			DrawDebugLine(GetWorld(), Anchor, Anchor + (Torque * 0.0001), FColor::Cyan, true, -1.0f, 2, 1.0f);


			AccumulilatedTorque += Torque;

			DrawDebugSphere(GetWorld(), Anchor, 10.0f, 16, FColor::Blue, true, 1.0, 1, 0.5f);
			DrawDebugSphere(GetWorld(), HitResult.Location, 10.0f, 16, FColor::Green, true, 1.0, 1, 0.5f);
			DrawDebugSphere(GetWorld(), RayEnd, 25.0f, 16, FColor::Purple, true, 1.0, 1, 1.0f);

			DrawDebugLine(GetWorld(), Anchor, HitResult.Location, FColor::Green, true, -1.0f, 1, 2.0f);

		}
		else
		{

			WheelMeshes[i]->SetWorldLocation(Anchor - (FVector::UpVector * MaxSuspesionLength));

			DrawDebugSphere(GetWorld(), Anchor, 50.0f, 16, FColor::Blue, true, 1.0, 1, 2.0f);
			DrawDebugSphere(GetWorld(), Anchor - (FVector::UpVector * MaxSuspesionLength), 50.0f, 16, FColor::Red, true, 1.0, 1, 1.0f);

			DrawDebugLine(GetWorld(), Anchor, Anchor - (FVector::UpVector * MaxSuspesionLength), FColor::Green, true, -1.0f, 1, 2.0f);

		}
	}

	//Vehicle Velo
	VehicleVelocity += (AccumilatedForce / VehicleMass) * DeltaTime;

	//UE_LOG(LogTemp, Log, TEXT("Accumulated Torque: %s"), *AccumulilatedTorque.ToString());
	//Vehicle T
	VehicleAngularVelocity += ((AccumulilatedTorque * 0.0001f) / InertialTensor) * DeltaTime;

	VehicleAngularVelocity *= 0.98f;


	if (VehicleAngularVelocity.Size() < 0.01f)
	{
		VehicleAngularVelocity = FVector::ZeroVector;
	}


	//UE_LOG(LogTemp, Log, TEXT("Vehicle Angular Velocity: %s"), *VehicleAngularVelocity.ToString());

	FRotator PhysicsRotation = GetActorRotation();
	FRotator SplineRotation = SplineTransform.GetRotation().Rotator();

	FRotator CombinedRotator;
	CombinedRotator.Pitch = PhysicsRotation.Pitch;
	CombinedRotator.Roll = PhysicsRotation.Roll;
	CombinedRotator.Yaw = SplineRotation.Yaw;


	//ActorRotation = this->GetActorRotation().Quaternion(); //ORIGINAL

	FQuat DeltaRotation = FQuat(VehicleAngularVelocity.GetSafeNormal(), VehicleAngularVelocity.Size() * DeltaTime);
	ActorRotation = (DeltaRotation * CombinedRotator.Quaternion()).GetNormalized();
	this->SetActorRotation(ActorRotation);

	if (ApplyLocomotion == true)
	{
		FVector ForwardVelocity = this->GetActorForwardVector() * VehicleAcc;  

		VehicleVelocity += ForwardVelocity;

		 
	}

	FVector CombinedLocation = FVector(SplineTransform.GetLocation().X, SplineTransform.GetLocation().Y, (this->GetActorLocation() += VehicleVelocity * DeltaTime).Z);

	this->SetActorLocation(CombinedLocation);
	//UE_LOG(LogTemp, Log, TEXT("Vehicle Spring Damper Velo %s"), *AccumilatedForce.ToString());

	BodyMesh->SetWorldLocation(CombinedLocation);

	DrawDebugBox(GetWorld(), (CombinedLocation + BoxLocation), BoxExtents, ActorRotation,FColor::Blue, true, 1.0f, 1, 1.0f);

	//Update Speed
	CurrentSpeed = (CombinedLocation - PreviousLocation).Size();
	PreviousLocation = CombinedLocation;

	DrawDebugSphere(GetWorld(), TargetTransform.GetLocation(), 50.0f, 16, FColor::Emerald, false, 1.0f, 1, 1.0f);

	UpdateWheelRotations(CurrentSpeed, TargetTransform, DeltaTime, this->GetActorRightVector(), CombinedLocation);


}

