// Fill out your copyright notice in the Description page of Project Settings.


#include "PlotGenerator.h"

// Sets default values for this component's properties
UPlotGenerator::UPlotGenerator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}



void UPlotGenerator::GeneratePlotAreas(TArray<FSampledSplinePoint> SplinePointData)
{

	UE_LOG(LogTemp, Warning, TEXT("Plot Generator - Updating Cell"));

	PlotIndexPoint.Empty();

	for (int i = 0; i < SplinePointData.Num(); i += 10)
	{

		if (i > SplinePointData.Num())
		{
			i = SplinePointData.Num() - 1;
		}

		DrawDebugPoint(GetWorld(), SplinePointData[i].Location,10.0f, FColor::Red, true, -1.0f, 2);

		PlotIndexPoint.Add(SplinePointData[i]);

	}



	for (int i = 0; i < PlotIndexPoint.Num() - 1; i++)
	{

		FVector ForwardOffset = PlotIndexPoint[i].Normal;

		FVector P0InsetLocation = PlotIndexPoint[i].Location + (PlotIndexPoint[i].Normal * 50);
		FVector P1InsetLocation = PlotIndexPoint[i + 1].Location - (PlotIndexPoint[i + 1].Normal * 50);

	
		//Create four points
		FVector P0 = P0InsetLocation + (PlotIndexPoint[i].RightVector * 500);
		FVector P1 = P1InsetLocation + (PlotIndexPoint[i + 1].RightVector * 500);
		
		FVector P2 = P0 + (PlotIndexPoint[i].RightVector * 2000);
		FVector P3 = P1 + (PlotIndexPoint[i + 1].RightVector * 2000);

		TArray<FVector> Locations;
		TArray<int> Indices;

		Locations.Add(P0);
		Locations.Add(P1);
		Locations.Add(P2);
		Locations.Add(P3);

		Indices.Add(0);
		Indices.Add(2);
		Indices.Add(1);
		Indices.Add(1);
		Indices.Add(2);
		Indices.Add(3);


		DrawDebugMesh(GetWorld(), Locations, Indices, FColor(255,112,52,200), true, -1.0f, 1);


	}


}




// Called when the game starts
void UPlotGenerator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlotGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

