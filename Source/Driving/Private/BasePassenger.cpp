// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePassenger.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ABasePassenger::ABasePassenger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollider"));
	CapsuleCollider->InitCapsuleSize(30.0f, 180.0f);
	RootComponent = CapsuleCollider;


	CharacterMesh = CreateDefaultSubobject<UStaticMeshComponent>("Character Mesh");
	CharacterMesh->SetupAttachment(CapsuleCollider);

}


void ABasePassenger::EnterVehicle()
{

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, "Passenger is Entering Vehicle");


	//Get our Vehicle / Pawn
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();

	USceneComponent* PlayerRootComponent = PlayerPawn->GetRootComponent();
	FTransform SocketTransform = PlayerRootComponent->GetSocketTransform("PassengerSocket");

	this->CapsuleCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	this->AttachToComponent(PlayerRootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	this->SetActorTransform(SocketTransform);


}


void ABasePassenger::ExitVehicle()
{


}



// Called when the game starts or when spawned
void ABasePassenger::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABasePassenger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

