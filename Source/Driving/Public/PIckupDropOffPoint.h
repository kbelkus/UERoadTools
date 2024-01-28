// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasePassenger.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "PIckupDropOffPoint.generated.h"


UENUM(BlueprintType)
enum class EPickupType : uint8 
{
	Pickup      UMETA(DisplayName = "Pickup Only"),
	Dropof      UMETA(DisplayName = "Dropoff Only"),
	Both        UMETA(DisplayName = "Both"),
};


UCLASS()
class DRIVING_API APIckupDropOffPoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APIckupDropOffPoint();

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* TriggerBox;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** called when something leaves the sphere component */
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);





protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Override Construction SCript
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* VisualMesh;
	UFUNCTION()
	void DisplayUI();
	UFUNCTION()
	void OnButtonPressed();
	UFUNCTION()
	void PickupPassenger();
	UFUNCTION()
	void RespondToConfirmButton();



public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Size = FVector(100,100,100);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int> AssociatedCharacters;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPickupType PickupTypeEnum;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isActive = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ABasePassenger* Passenger; 
	UPROPERTY(EditAnywhere)
	bool isInsideTriggerBox = false;
	//AWheeledVehiclePawn* WheeledVehiclePawn


	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
