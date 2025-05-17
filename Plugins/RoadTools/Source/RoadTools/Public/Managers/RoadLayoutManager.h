// Copyright Kieran Belkus 2025
// Utility Manager for automatically connecting RoadSplines together. For now, its entirely brute force, but will move to a grid later

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadLayoutManager.generated.h"

class ALaneSpline;

USTRUCT(BlueprintType)
struct FRoadNetworkLanes
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<ALaneSpline> LaneSpline;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SplineLength;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FBox BoundingBox;

	FRoadNetworkLanes()
	{
	}

};

USTRUCT(BlueprintType)
struct FVehicleSpawnPoints
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector SpawnLocation = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FVehicleSpawnPoints()
	{
	}

};


UCLASS()
class ROADTOOLS_API ARoadLayoutManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoadLayoutManager();

	UPROPERTY(EditAnywhere)
	float SplineSearchDistance = 100.0f;

	UPROPERTY(VisibleAnywhere)
	TArray<FRoadNetworkLanes> LaneSplines;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(CallInEditor)
	void BuildRoadNetwork();

	UFUNCTION()
	TArray<ALaneSpline*> GetAllSceneSplines(UWorld* InWorldContext);

	UFUNCTION()
	TArray<FRoadNetworkLanes> BuildRoadNetworkSceneData(TArray<ALaneSpline*> InLaneSplines);

	UFUNCTION()
	void ConnectAllLaneSplines(TArray<ALaneSpline*> InLaneSplines);

	UFUNCTION()
	FRoadNetworkLanes GetRoadSplineData(int InID);

	UFUNCTION()
	ALaneSpline* LoadLaneSpline(FRoadNetworkLanes InRoadNetworkLane);

	UFUNCTION()
	FRoadNetworkLanes GetNearestRoadSpline(const FVector InLocation, bool& FoundSpline, float& NearestDistanceAlongSpline);

	//Debug Functions for Testing
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Debug Tests")
	FVector InTestLocation;
	UFUNCTION(CallInEditor, Category="Debug Tests")
	void TestGetNearestLaneSplineLocation();

};
