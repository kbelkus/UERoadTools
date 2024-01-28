// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasePassenger.generated.h"


//What is this actor currently doing so we can track their status from the world manager
UENUM(BlueprintType)
enum class EStatus : uint8
{
	Waiting			UMETA(DisplayName = "Waiting"),
	Riding			UMETA(DisplayName = "Riding"),
	Completed       UMETA(DisplayName = "Completed"),
};

//Character Emotional State Currently - Derive this from emotion factors in the futre.
//Use this to drive dialog options and choices and tip amount
UENUM(BlueprintType)
enum class EEmotionalStatus : uint8
{
	Neutral      UMETA(DisplayName = "Neutral"),
	Angry		 UMETA(DisplayName = "Angry"),
	Happy        UMETA(DisplayName = "Happy"),
	Sad			 UMETA(DisplayName = "Sad"),
};


UCLASS()
class DRIVING_API ABasePassenger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePassenger();

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* CapsuleCollider;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* CharacterMesh;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere)
	int CharacterID; //Set from Character DB (or assign random id for generic passenger)
	UPROPERTY(EditAnywhere)
	FString CharacterName;
	UPROPERTY(EditAnywhere)
	int DestinationID;
	
	UFUNCTION()
	void EnterVehicle();
	UFUNCTION()
	void ExitVehicle();

	


};
