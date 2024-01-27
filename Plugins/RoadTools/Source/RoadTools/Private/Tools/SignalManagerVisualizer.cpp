// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "Tools/SignalManagerVisualizer.h"
#include "ComponentVisualizer.h"
#include "GameFramework/Actor.h"
#include "JunctionSignalController.h"
#include "SignalControllerComponent.h"

void FSignalManagerVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView*, FPrimitiveDrawInterface * PDI)
{
	const USignalControllerComponent* SignalControllerComponent = Cast<USignalControllerComponent>(Component);


	if (!SignalControllerComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("COMPONENT NOT FOUND"));
		return;
	}


	const AActor* OwnerActor = Component->GetOwner();

	const AJunctionSignalController* SignalController = Cast<AJunctionSignalController>(OwnerActor);

	if (!SignalController)
	{
		return;
	}


	const FVector OwnerLocation = OwnerActor->GetActorLocation();


	//PDI->DrawLine(OwnerLocation, SignalControllerComponent->EndPosition, FLinearColor::Red, ESceneDepthPriorityGroup::SDPG_Foreground, 10.0f);
	//PDI->DrawLine(OwnerLocation, SignalControllerComponent->StartPosition, FLinearColor::Green, ESceneDepthPriorityGroup::SDPG_Foreground, 10.0f);

	//FMeshBatch SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT(""));


	//Get all Phase Connected Actors

	if (SignalController->Phases.Num() > 0)
	{

		TArray<ALaneSpline*>ProceedLanes = SignalController->Phases[0].LaneGroupProceed;

		if (ProceedLanes.Num() > 0)
		{
			for (const ALaneSpline* Spline : ProceedLanes)
			{
				if (Spline)
				{
					USplineComponent* LaneSpline = Spline->ReturnSpline();

					FVector EndLocation = LaneSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

						PDI->DrawLine(OwnerLocation, EndLocation, FLinearColor::Red, ESceneDepthPriorityGroup::SDPG_Foreground, 10.0f);
						//PDI->DrawSphere

				}


			}


		}

	}

}


