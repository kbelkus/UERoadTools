// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoadSurface.h"
#include "JunctionSurface.generated.h"

class UProceduralMeshComponent;

USTRUCT() //DEP
struct FJunctionRoadProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
		int RoadID;
	UPROPERTY(EditAnywhere)
		float LaneCount;
	UPROPERTY(EditAnywhere)
		float RoadWidth;
	UPROPERTY(EditAnywhere)
		float LeftAngle;
	UPROPERTY(EditAnywhere)
		float RightAngle;
};


//These are Points that come from the Road Manager
//TO DO: Find a way to make our own custom data type we can share between all classes
USTRUCT()
struct FPointsFromRoadManager
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector WorldSpacePosition;
	UPROPERTY(EditAnywhere)
	FVector WorldSpaceNormal;
	UPROPERTY(EditAnywhere)
	FVector WorldSpaceRightVector;
	UPROPERTY(EditAnywhere)
	ARoadSurface* ConnectedRoad;

	FPointsFromRoadManager()
	{

	}
};



USTRUCT()
struct FJunctionPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int RoadID;
	UPROPERTY(EditAnywhere)
	FVector Position;
	UPROPERTY(EditAnywhere)
	int JunctionType;
	UPROPERTY(EditAnywhere)
	float AngleFromCenter;
	UPROPERTY(EditAnywhere)
	int Index;

	FJunctionPoint()
	{
		RoadID = int(0);
		Position = FVector(0, 0, 0);
		JunctionType = int(0);
		AngleFromCenter = 0.0f;
		Index = int(0);
	}

	FORCEINLINE bool operator()(const FJunctionPoint& A, const FJunctionPoint& B) const
	{
		return A.AngleFromCenter > B.AngleFromCenter;
	}

};

//Store the data about each of our junction parts so we can use this data to generate the mesh later
//and easily expose how we want to control the visual look of the junction
USTRUCT()
struct FJunctionData
{
	GENERATED_BODY()

	UPROPERTY()
	int SectionCount;
	int SectionID;
	TArray<FVector> PointPosition;
	TArray<int> PointID;
	int JunctionType; //0 - Stop Section 1 - Pass Through
	FVector SectionCenterPosition;


	FJunctionData()
	{
		SectionCount = int(0);
		SectionID = int(0);
		JunctionType = int(0);
		SectionCenterPosition = FVector(0, 0, 0);
	}

};



UCLASS()
class ROADTOOLS_API AJunctionSurface : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJunctionSurface();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//User Input Data
	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
	TArray<FVector> JunctionEndPoints;
	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
	FVector JunctionCenter;

	//Preset Vars
	TArray<FColor> JunctionColorCodes = { FColor::Red, FColor::Green, FColor::Blue, FColor::Yellow, FColor::Turquoise };

	//Functions
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void GenerateJunctionPoints();
	UFUNCTION()
	void DrawRoadPoints();
	void DrawJunctionShape();
	void DrawJunctionVolume();
	
	//Generated Var
	//TArray<FJunctionPoint> JunctionPoints;

	//Points to Desribe where on the RoadCurve does the junction Start
	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
	TArray<FVector> JunctionCenterPoints;
	//Points to describe where in worldspace does the Volume of the junction exist
	UPROPERTY(EditAnywhere)
	TArray<FVector> JunctionVolumePoints;

	/*
	UFUNCTION()
	bool CompareFloats(const FJunctionPoint& A, const FJunctionPoint& B)
	{
		return A.AngleFromCenter > B.AngleFromCenter;
	}

	UFUNCTION()
	void SortPoints(TArray<FJunctionPoint>& PointArray);
	*/



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UProceduralMeshComponent* JunctionSurface;

public:

	//Properties That are Set by RoadManager
	UPROPERTY(EditAnywhere)
	int RoadCount;
	UPROPERTY(EditAnywhere)
	int JunctionID;
	UPROPERTY(EditAnywhere)
	FVector Location;
	UPROPERTY(EditAnywhere)
	TArray<ARoadSurface*> ConnectedRoads;
	UPROPERTY(EditAnywhere)
	TArray<int> ConnectedRoadPoints;
	UPROPERTY(EditAnywhere)
	TArray<FJunctionPoint> JunctionPoints;
	UPROPERTY(EditAnywhere)
	TArray<FPointsFromRoadManager> RoadManagerPoints;

	UFUNCTION()
	void BuildJunction();

	UPROPERTY(EditAnywhere)
	TArray<FJunctionRoadProperties> JunctionRoads;

};
