// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


class ROADTOOLS_API MathHelperFunctions
{
public:
	MathHelperFunctions();
	~MathHelperFunctions();


	UFUNCTION()
	static float SimpleLerpTest(float x, float y);
	
	//CURVE EASING FUNCTIONS
	UFUNCTION()
	static float CustomEaseInOutQuad(float start, float end, float t); //Ease InOut between a start end range
	static float EaseInOutQuad(float Start, float End, float t);

	//CURVE FUNCTIONS
	static FVector BezierCurvePosition(FVector P0, FVector P1, FVector P2, float t);


};
