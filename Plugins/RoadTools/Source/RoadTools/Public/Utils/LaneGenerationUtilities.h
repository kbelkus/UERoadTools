// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"

class ALaneSpline;
class USplineComponent;
class RoadSurface;


class ROADTOOLS_API LaneGenerationUtilities
{

public:
	LaneGenerationUtilities();
	~LaneGenerationUtilities();


	TObjectPtr<ALaneSpline> GenerateLaneData();

};
