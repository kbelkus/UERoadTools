// Fill out your copyright notice in the Description page of Project Settings.

#include "JunctionSignalController.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "JunctionSurface.h"
#include "LaneSpline.h"

// Sets default values
AJunctionSignalController::AJunctionSignalController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComponent);

}

void AJunctionSignalController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Center = this->GetActorLocation();
	Extent = this->GetActorScale();

	if (DrawDebug)
	{
		FlushPersistentDebugLines(GetWorld());
		DrawPhaseDebug();
	}
}

/*
void AJunctionSignalController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FlushPersistentDebugLines(GetWorld());
	DrawPhaseDebug();
}
*/

void AJunctionSignalController::DrawPhaseDebug()
{

	//Draw Arrows At Lane Start
	if (Phases.Num() == 0)
	{
		return;
	}

	SignalDebugObjects.Empty();
	LaneProceedPointPositions.Empty();

	//Do this to stop crashes
	int ClampedPhaseIndex = FMath::Clamp(DebugPhaseIndex, 0, Phases.Num() - 1);

	FSignalPhase CurrentPhase = Phases[ClampedPhaseIndex];

	if (CurrentPhase.LaneGroupProceed.Num() > 0 && CurrentPhase.LaneGroupStop.Num() > 0)
	{

		//Draw Procced Arrows
		for (int i = 0; i < CurrentPhase.LaneGroupProceed.Num(); i++)
		{
			if (CurrentPhase.LaneGroupProceed[i])
			{

				USplineComponent* CurrentLaneComponent = CurrentPhase.LaneGroupProceed[i]->ReturnSpline();
				FVector Location = CurrentLaneComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World) + DebugSignalHeight;
				FVector Direction = CurrentLaneComponent->GetDirectionAtSplinePoint(0, ESplineCoordinateSpace::World);

				//Append Some Lane Point Positions for easier debugging
				for (int j = 0; j < 20; j++)
				{
					LaneProceedPointPositions.Add(CurrentLaneComponent->GetLocationAtTime(j * 0.05, ESplineCoordinateSpace::World, false));
				}

				FSignalDebugRenderer NewDebugObject;

				NewDebugObject.Colour = FColor::Green;
				NewDebugObject.Position = Location + (-Direction * 200.0f);
				NewDebugObject.Direction = Direction;

				SignalDebugObjects.Add(NewDebugObject);
			}			
		}

		for (int i = 0; i < CurrentPhase.LaneGroupStop.Num(); i++)
		{

			if (CurrentPhase.LaneGroupStop[i])
			{

				USplineComponent* CurrentLaneComponent = CurrentPhase.LaneGroupStop[i]->ReturnSpline();
				FVector Location = CurrentLaneComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World) + DebugSignalHeight;
				FVector Direction = CurrentLaneComponent->GetDirectionAtSplinePoint(0, ESplineCoordinateSpace::World);

				FSignalDebugRenderer NewDebugObject;

				NewDebugObject.Colour = FColor::Red;
				NewDebugObject.Position = Location + (-Direction * 200.0f);
				NewDebugObject.Direction = Direction;

				SignalDebugObjects.Add(NewDebugObject);

				//FVector Position = 
				//DrawDebugCone(GetWorld(), Location, Direction, 500.0f, 10.0f, 3.0f, 32, FColor::Red, true, -1.0f, ESceneDepthPriorityGroup::SDPG_Foreground, 1.0f);
			}
		}


		//Draw Stop Arrows
		DrawTrafficDebug();
		//DrawLaneDebugPoints();

	}
}

void AJunctionSignalController::DrawLaneDebugPoints()
{
	if (LaneProceedPointPositions.Num() > 0)
	{

		for (int i = 0; i < LaneProceedPointPositions.Num(); i++)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Junction Signal Controller: DrawLaneDebugPoints: Has Points"));
			DrawDebugPoint(GetWorld(), LaneProceedPointPositions[i], 20.0f, FColor::Green, true, -1.0f, 1);
			//DrawDebugPoint(GetWorld(), StartPosition, 20.0f, DebugColours[i], true, -1.0f, 10);
		}
	}
}


void AJunctionSignalController::DrawTrafficDebug()
{
	if (SignalDebugObjects.Num() == 0)
	{
		return;
	}

	for (int i = 0; i < SignalDebugObjects.Num(); i++)
	{

		DrawDebugCone(GetWorld(), SignalDebugObjects[i].Position, SignalDebugObjects[i].Direction, Length, AngleWidth, AngleHeight, 32, SignalDebugObjects[i].Colour, true, -1.0f, 2, 3.0f);

		DrawDebugSphere(GetWorld(), SignalDebugObjects[i].Position, 100.0f, 10, SignalDebugObjects[i].Colour, true, -1.0f, 2.0f, 3.0f);
	}

	if (LaneProceedPointPositions.Num() > 0)
	{

		for (int i = 0; i < LaneProceedPointPositions.Num(); i++)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Junction Signal Controller: DrawLaneDebugPoints: Has Points"));
			DrawDebugPoint(GetWorld(), LaneProceedPointPositions[i], 20.0f, FColor::Green, true, -1.0f, 1);
			//DrawDebugPoint(GetWorld(), StartPosition, 20.0f, DebugColours[i], true, -1.0f, 10);
		}
	}

}



void AJunctionSignalController::DrawDebugBounds()
{
	//DrawDebugBox(GetWorld(), Center, Extent, FColor::Blue, true, 1000.0f, 1, 10.0f);
	//DrawDebugSphere(GetWorld(), Center, 100.0f, 32, FColor::Red, false, -1.0f, 1, 10.0f);
	//DrawDebugPoint(GetWorld(), Center, 20.0f, FColor::Red, true, -1.0f, 10);

}



//When called loop through all connected splines and set their lane status
void AJunctionSignalController::UpdateAllConnectedJunctions()
{

}

void AJunctionSignalController::RebuildSignalController()
{
	if (ConnectedJunctionSurface == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("JunctionSignalController could not be built, there is no attached controller, please add one to: %s"), *this->GetName());
		return;
	}

	TObjectPtr<AJunctionSurface> JunctionSurface = ConnectedJunctionSurface.LoadSynchronous();
	Phases.Empty();

	if (JunctionSurface)
	{
		//Get All Related Splines
		TArray<TObjectPtr<ALaneSpline>> RelatedLaneSplines;

		for (TSoftObjectPtr<ALaneSpline> LaneSpline : JunctionSurface->GeneratedLaneSplines)
		{
			TObjectPtr<ALaneSpline> LoadedLaneSpline = LaneSpline.LoadSynchronous();

			if (LoadedLaneSpline && LoadedLaneSpline->LaneSplineType == ELaneSplineType::JUNCTION)
			{
				RelatedLaneSplines.Add(LoadedLaneSpline);
			}
		}

		int MaximumPhaseCount = GetMaximumPhaseCount(JunctionSurface);
		//Bundle Them into their correct categories

		UE_LOG(LogTemp, Log, TEXT("Junction Signal Controller has this many phases: %i"), MaximumPhaseCount);

		//Rebuild The Signal Controller Phases!
		for (int i = 0; i < MaximumPhaseCount; i++)
		{
			FSignalPhase NewSignalPhase;
			NewSignalPhase.PhaseLength = 20.0f; //MOVE THIS

			for (TObjectPtr<ALaneSpline> CurrentSpline : RelatedLaneSplines)
			{
				if (CurrentSpline->SignalActivePhase.Contains(i))
				{
					NewSignalPhase.LaneGroupProceed.Add(CurrentSpline);
				}
				else
				{
					NewSignalPhase.LaneGroupStop.Add(CurrentSpline);
				}
			}

			Phases.Add(NewSignalPhase);
		}
	}

	return;
}


//During Gameplay Update Lanes
void AJunctionSignalController::UpdateLanes(int Phase)
{

	//Draw Arrows At Lane Start
	if (Phases.Num() == 0)
	{
		return;
	}

	for (int i = 0; i < Phases[Phase].LaneGroupProceed.Num(); i++)
	{
		if (Phases[Phase].LaneGroupProceed[i])
		{
			Phases[Phase].LaneGroupProceed[i]->LaneStatus = 1;
		}
	}


	for (int i = 0; i < Phases[Phase].LaneGroupStop.Num(); i++)
	{
		if (Phases[Phase].LaneGroupStop[i])
		{
			Phases[Phase].LaneGroupStop[i]->LaneStatus = 0;
		}
	}
}

//With Update our Phase Index and Call our Update Lanes Function
void AJunctionSignalController::UpdatePhaseTimer()
{

	UpdateSignals = true;
	GetWorldTimerManager().ClearTimer(PhaseTimeHandle);

}



// Called when the game starts or when spawned
void AJunctionSignalController::BeginPlay()
{
	Super::BeginPlay();

	if (isEnabled == true)
	{
		int ClampedPhaseIndex = FMath::Clamp(PhaseIndex, 0, Phases.Num() - 1);
		UpdateLanes(ClampedPhaseIndex);
	}
	
	UpdateSignals = true;
}

// Called every frame
void AJunctionSignalController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Allow Disabling of this junction actor for debugging / Gameplay
	if (isEnabled == true)
	{
		int ClampedPhaseIndex = FMath::Clamp(DebugPhaseIndex, 0, Phases.Num() - 1);
		UpdateLanes(ClampedPhaseIndex);
	}

	//Junction Timer
	if (UpdateSignals == true)
	{
		UpdateSignals = false;

		int NextPhase = (PhaseIndex + 1) % Phases.Num();
		PhaseIndex = NextPhase;
		UpdateLanes(PhaseIndex);
		DebugPhaseIndex = PhaseIndex;
		FlushPersistentDebugLines(GetWorld());
		DrawPhaseDebug();

		UE_LOG(LogTemp, Warning, TEXT("Junction Signal Controller: Update Signal Timer %i"), PhaseIndex);
		GetWorld()->GetTimerManager().SetTimer(PhaseTimeHandle, this, &AJunctionSignalController::UpdatePhaseTimer, Phases[PhaseIndex].PhaseLength, false);
	}

}

//Loop through each phase value and get max
int AJunctionSignalController::GetMaximumPhaseCount(AJunctionSurface* InConnectedJunctionSurface)
{
	int MaximumPhaseCount = 0;

	for (int i = 0; i < InConnectedJunctionSurface->JunctionPoints.Num(); i++)
	{
		for (int j = 0; j < InConnectedJunctionSurface->JunctionPoints[i].LeftLanes.Num(); j++)
		{
			if (!InConnectedJunctionSurface->JunctionPoints[i].LeftLanes[j].SignalActivePhase.IsEmpty())
			{
				MaximumPhaseCount = FMath::Max(MaximumPhaseCount, InConnectedJunctionSurface->JunctionPoints[i].LeftLanes[j].SignalActivePhase[0]);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Junction Surface:: GetMaximumPhaseCount - Junction Point %i has no signal phase set"), i);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("This junction has a maximum phase count of: %i"), MaximumPhaseCount);

	//Do this to stop any accidental access issues
	if (MaximumPhaseCount == 0)
	{
		return 0;
	}


	return MaximumPhaseCount + 1;
}
