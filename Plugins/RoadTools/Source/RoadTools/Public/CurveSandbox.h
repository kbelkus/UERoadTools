 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "CurveSandbox.generated.h"


USTRUCT()
struct FSimplePolygon
{
	GENERATED_BODY()

	UPROPERTY()
	int SplineID;
	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
	TArray<FVector> VertexPosition;

	FSimplePolygon()
	{
		SplineID = int(0);
	}
};

//For Each Intersection we find - store the Splines Involved, their KeyPointIDs and Location of Intersection
USTRUCT()
struct FIntersections
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<USplineComponent*> SplineReference;
		
	UPROPERTY(EditAnywhere)
	FVector IntersectPosition;

	UPROPERTY(EditAnywhere)
	TArray<int> PointID; //This should be in the same order as the SplineReference


	FIntersections()
	{
		//SplineID = int(0);
	}
};



UCLASS()
class ROADTOOLS_API ACurveSandbox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACurveSandbox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Override Construction SCript
	virtual void OnConstruction(const FTransform& Transform) override;

	//Functions
	UFUNCTION()
	void GenerateSegmentPolygon(int SegmentId);

	UFUNCTION()
	void GenerateFatLine(int SegmentId);

	UFUNCTION()
	void FindKeyIntersection();
	UFUNCTION()
	void DebugDrawIntersections();




	//Preset Vars
	TArray<FColor> JunctionColorCodes = { FColor::Red, FColor::Green, FColor::Blue, FColor::Yellow, FColor::Turquoise };

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	UPROPERTY(VisibleAnywhere, Category = "Spline")
		USplineComponent* Spline1;
	UPROPERTY(VisibleAnywhere, Category = "Spline")
		USplineComponent* Spline2;
	UPROPERTY(EditAnywhere, Category = "Spline")
		TArray<USplineComponent*> SplineLibrary;
	UPROPERTY(EditAnywhere, Category = "Spline")
		TArray<FVector> VecArray;
	UPROPERTY(EditAnywhere, Category = "Spline")
	TArray<FIntersections> Intersections;




};
