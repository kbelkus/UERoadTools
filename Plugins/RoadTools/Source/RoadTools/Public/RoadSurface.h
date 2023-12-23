// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "RoadSurface.generated.h"


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
	int LengthResolution;
	UPROPERTY(EditAnywhere)
	int WidthResolution;

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
	void GenerateTriangles();
	UFUNCTION()
	void PointsAlongSpline(float StartDistance, float EndDistance);

	//Override Construction SCript
	virtual void OnConstruction(const FTransform& Transform) override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	UProceduralMeshComponent* RoadSurface;

public:
	UPROPERTY(VisibleAnywhere, Category = "Spline")
	USplineComponent* CenterSpline;

	//User Vars
	UPROPERTY(EditAnywhere)
	float RoadWidth;

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
