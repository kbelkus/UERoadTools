// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "RoadSurface.generated.h"


//Store our data when sampled from a point in this struct 
USTRUCT()
struct FSampledSplinePoint
{
	GENERATED_BODY()
	FVector Location;
	FVector RightVector;
	FVector Normal;

	FSampledSplinePoint()
	{
	}
};

USTRUCT()
struct FLaneMarking
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float MarkingOffset = 0.0;
	UPROPERTY(EditAnywhere)
	float MarkingWidth = 10.0f;
	UPROPERTY(EditAnywhere)
	UMaterialInterface* MarkingMaterial;
	UPROPERTY(EditAnywhere)
	FVector2D UVTiling = FVector2D(1.0f, 1.0f);
	UPROPERTY(EditAnywhere)
	FVector2D UVOffset = FVector2D(0.0f, 0.0f);


	FLaneMarking()
	{

	}

};



USTRUCT()
struct FLaneData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float LaneWidth = 350.0f;
	UPROPERTY(EditAnywhere)
	float Start = 0.0f;
	UPROPERTY(EditAnywhere)
	float End = 100.0f;
	UPROPERTY(EditAnywhere)
	int LaneType;
	UPROPERTY(EditAnywhere)
	int LaneWidthResoution;
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;
	UPROPERTY(EditAnywhere)
	FVector2D UVTiling = FVector2D(0.3f,1.0f);
	UPROPERTY(EditAnywhere)
	FVector2D UVOffset = FVector2D(0.0f,0.0f);
	UPROPERTY(EditAnywhere)
	TArray<FLaneMarking> LaneMarkings;

	FLaneData()
	{
		Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/Road_Surface_Master_Inst.Road_Surface_Master_Inst"));

	}
};


USTRUCT()
struct FLaneSection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FLaneData LaneProfile;
	UPROPERTY(EditAnywhere)
	float Start = 0.0f;
	UPROPERTY(EditAnywhere)
	float End = 100.0f;

	FLaneSection()
	{

	}

};


USTRUCT()
struct FRoadProfile
{
	GENERATED_BODY()
	TArray<float> Offset;

	FRoadProfile()
	{

	}
};


class UProceduralMeshComponent;

UCLASS()
class ROADTOOLS_API ARoadSurface : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoadSurface();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	bool UseAdvancedRoadDesigner = false; //Backwards Compat Support
	UPROPERTY(EditAnywhere)
	bool CreateLaneMarkings = false;

	UPROPERTY(EditAnywhere)
	int LengthResolution = 400;
	UPROPERTY(EditAnywhere)
	int WidthResolution = 5;

	//Shared Vars
	int SplinePointCount;

	//USER: Test RoadInputs
	UPROPERTY(Meta = (MakeEditWidget = true))
	TArray<FVector> RoadPoints;

	//GENERATED: Data needed for Road Mesh Calculation
	TArray<FVector> RoadPointNormal;
	TArray<FVector> RoadPointTangent;
	//UPROPERTY(Meta = (MakeEditWidget = true))
	TArray<FVector> VertexPositions;
	TArray<int> TriangleIndices;
	//UPROPERTY(Meta = (MakeEditWidget = true))
	TArray<FVector> SplinePositions;

	//Functions
	UFUNCTION()
	void GenerateMeshPoints();
	UFUNCTION()
	void BuildAdvancedRoad(TArray<FLaneData> Lanes, int LaneIndex, int offset);
	UFUNCTION()
	void GenerateAdvancedMeshPoints(TArray<FLaneData> Lanes, int LaneIndex);
	UFUNCTION()
	void GenerateLaneMarkings(TArray<FLaneData> Lanes, int LaneIndex);
	UFUNCTION()
	void GenerateTriangles();
	UFUNCTION()
	void GenerateAdvancedTriangles();

	UFUNCTION()
	void PointsAlongSpline(float StartDistance, float EndDistance);
	UFUNCTION()
	void DebugDrawVertices();

	//Override Construction SCript
	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	UProceduralMeshComponent* RoadSurface;
	UProceduralMeshComponent* LaneMarkingSurface;

public:


	UPROPERTY(EditAnywhere)
	bool GeneratePlots = false;

	UPROPERTY(VisibleAnywhere, Category = "Spline")
	USplineComponent* CenterSpline;

	//V2 Controls
	UPROPERTY(EditAnywhere)
	TArray<FLaneData> LeftLanes;
	UPROPERTY(EditAnywhere)
	TArray<FLaneData> RightLanes;
	TArray<FSampledSplinePoint>SplinePointData;
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> AdvancedVertexPositions;
	TArray<FVector2D> AdvancedVertexUV;
	TArray<FVector> LaneMarkingVertices;
	TArray<FVector2D> LaneMarkingUVs;

	int ProfileVertexCount;
	float SplineLength;
	FVector RoadActorLocation;

	//User Vars
	
	UPROPERTY(EditAnywhere)
	float RoadWidth = 350.0f;

	//Comes from Road Manager
	UPROPERTY(EditAnywhere)
	TArray<float>IntersectPoints;
	UPROPERTY(EditAnywhere)
	TArray<int>JunctionPoints;
	UPROPERTY(EditAnywhere)
	bool EditorMode = true;
	UFUNCTION()
	void BuildRoad();

	UFUNCTION()
	void ProjectSplinePoints();
	UPROPERTY(EditAnywhere)
	bool ProjectSpline = false;




};
