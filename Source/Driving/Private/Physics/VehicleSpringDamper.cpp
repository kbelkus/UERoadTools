// Fill out your copyright notice in the Description page of Project Settings.


#include "Physics/VehicleSpringDamper.h"

// Sets default values
AVehicleSpringDamper::AVehicleSpringDamper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent->SetMobility(EComponentMobility::Movable);
	SetRootComponent(RootComponent);

}

// Called when the game starts or when spawned
void AVehicleSpringDamper::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < 4; i++)
	{
		OldDistance[i] = MaxSuspesionLength;
	}

	CarBodyTransform = this->GetActorTransform();
	
}

// Called every frame
void AVehicleSpringDamper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//FlushPersistentDebugLines(GetWorld());

	VehicleVelocity += FVector(0, 0, -980.0f) * DeltaTime;
	FVector AccumilatedForce = FVector::ZeroVector;

	for (int i = 0; i < 4; i++)
	{
		
		//RayCast for Each Wheel
		FCollisionQueryParams CollisionParms;

		FVector Anchor = this->GetActorLocation() + WheelOffset[i];
		FVector RayEnd = Anchor - ((FVector::UpVector * MaxSuspesionLength) + FVector(0,0,50.0f));

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Anchor, RayEnd, ECC_WorldStatic, CollisionParms);

		if (bHit)
		{
			FVector HitLocation = HitResult.Location;

			FVector SuspensionDirection = (Anchor - HitLocation).GetSafeNormal();

			float CurrentLength = (Anchor - HitLocation).Size();
			float Compression = RestLength - CurrentLength;
			float VelocityAlongSpring = (OldDistance[i] - CurrentLength) / DeltaTime;

			UE_LOG(LogTemp, Log, TEXT("Compression: %f, VelocityAlongSpring: %f"), Compression, VelocityAlongSpring);

			UE_LOG(LogTemp, Log, TEXT("Velocity Along Spring %f"), VelocityAlongSpring);
			OldDistance[i] = CurrentLength;

			FVector SpringForce = SpringStiffness * Compression * SuspensionDirection;
			FVector DampingForce = DampingCoefficient * VelocityAlongSpring * SuspensionDirection;

			FVector TotalForce = SpringForce + DampingForce;
			
			if (FMath::Abs(VelocityAlongSpring) < 10.0f && Compression < 60.0f)
			{
				TotalForce = FVector::ZeroVector;
			}

			
			AccumilatedForce += TotalForce;

			//UE_LOG(LogTemp, Log, TEXT("Wheel %d | Compression: %.2f | Velocity: %.2f | SpringForce: %s | DampingForce: %s | TotalForce: %s"),
			//	i,
			//	Compression,
			//	VelocityAlongSpring,
			//	*SpringForce.ToString(),
			//	*DampingForce.ToString(),
			//	*TotalForce.ToString()
			//);
			

			DrawDebugSphere(GetWorld(), Anchor, 50.0f, 16, FColor::Blue, true, 1.0, 1, 2.0f);
			DrawDebugSphere(GetWorld(), HitResult.Location, 50.0f, 16, FColor::Green, true, 1.0, 1, 2.0f);
			DrawDebugSphere(GetWorld(), RayEnd, 25.0f, 16, FColor::Purple, true, 1.0, 1, 2.0f);

			DrawDebugLine(GetWorld(), Anchor, HitResult.Location, FColor::Green, true, -1.0f, 1, 2.0f);

		}
		else
		{
			DrawDebugSphere(GetWorld(), Anchor, 50.0f, 16, FColor::Blue, true, 1.0, 1, 2.0f);
			DrawDebugSphere(GetWorld(), Anchor - (FVector::UpVector * MaxSuspesionLength), 50.0f, 16, FColor::Red, true, 1.0, 1, 2.0f);

			DrawDebugLine(GetWorld(), Anchor, Anchor - (FVector::UpVector * MaxSuspesionLength), FColor::Green, true, -1.0f, 1, 2.0f);

		}
	}

	VehicleVelocity += (AccumilatedForce / VehicleMass) * DeltaTime;

	this->SetActorLocation(this->GetActorLocation() += VehicleVelocity * DeltaTime);
	DrawDebugBox(GetWorld(), this->GetActorLocation() += VehicleVelocity * DeltaTime, FVector(300, 200, 50), FColor::Blue, true, 1.0f, 1, 2.0f);

}

