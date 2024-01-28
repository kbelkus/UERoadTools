// Fill out your copyright notice in the Description page of Project Settings.


#include "PIckupDropOffPoint.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"

// Sets default values
APIckupDropOffPoint::APIckupDropOffPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->InitBoxExtent(Size);
	TriggerBox->SetCollisionProfileName("PickUpCollisionBox");
	TriggerBox->SetLineThickness(2.0f);
	TriggerBox->SetupAttachment(GetRootComponent());


	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>("Visual Mesh");
	VisualMesh->SetupAttachment(TriggerBox);


	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APIckupDropOffPoint::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &APIckupDropOffPoint::OnOverlapEnd);

}


void  APIckupDropOffPoint::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Entered Box");

	if (Passenger == nullptr)
	{
		return;
	}

	FString CharacterName = "Pickup:" + Passenger->CharacterName;
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, CharacterName);

	//class OverlappingActor = OtherActor->GetClass();

	FString ActorName = OtherActor->GetName();
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, ActorName);

	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (PlayerPawn == OtherActor)
	{

		isInsideTriggerBox = true;

	}

	//Pickup Character Logic Here
	//Display a UI
	//Look for Controller Input + Timer
	//Trigger Character to Move to Car
	DisplayUI();


}


void APIckupDropOffPoint::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Exited Box");
	isInsideTriggerBox = false;
}


void APIckupDropOffPoint::OnConstruction(const FTransform& RootTransform)
{
	Super::OnConstruction(RootTransform);


	TriggerBox->SetBoxExtent(Size);

}

void APIckupDropOffPoint::RespondToConfirmButton()
{

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "KeyDown");
}



//Display Worldspace UI 
void APIckupDropOffPoint::DisplayUI()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "SHOW UI");
}


void APIckupDropOffPoint::OnButtonPressed()
{

}

//If criteria is met, pickup Passenger
void APIckupDropOffPoint::PickupPassenger()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "PASSENGER PICKUP CONFIRMED");
}



// Called when the game starts or when spawned
void APIckupDropOffPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APIckupDropOffPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isInsideTriggerBox == true && isActive == true)
	{

		APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

		if (PlayerController->IsInputKeyDown(EKeys::F) == true)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, "KEY IS PRESSED INSIDE BOX");

			//Make Passenger Enter Vehicle
			if (Passenger != nullptr)
			{
				Passenger->EnterVehicle();
				isActive = false;

			}

		}

	}

}

