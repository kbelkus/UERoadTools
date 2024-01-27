// Fill out your copyright notice in the Description page of Project Settings.

#include "MathHelperFunctions.h"
#include "Math/UnrealMathUtility.h"




float MathHelperFunctions::SimpleLerpTest(float x, float y)
{
	return FMath::Lerp(x, y, 0.5);
}


float MathHelperFunctions::CustomEaseInOutQuad(float start, float end, float t)
{

	float NormalizedT = (t - start) / (end - start);

	if (t >= start && t <= end)
	{
		if (NormalizedT < 0.5f)
		{
			return 0.5f * NormalizedT * NormalizedT;
		}
		else
		{
			return -0.5f * (NormalizedT * (NormalizedT - 2) - 1);
		}
	}


	if (t < start)
	{
		return t / start;
	}
	else
	{
		return 1 + (t - 1) / (1 - end);
	}
}

float MathHelperFunctions::EaseInOutQuad(float Start, float End, float t)
{
	t = (t - Start) / (End - Start);

	float b = 0;
	float c = 1;
	float d = 1;

	//if ((t /= d / 2) < 1.0f)
	//{
	//	return FMath::Clamp(c / 2 * t * t + b,0.0f,1.0f);
	//}
	//return FMath::Clamp( - c / 2 * ((--t) * (t - 2) - 1) + b,0.0f,1.0f);

	//return FMath::InterpStep(InValue, 0.5) * InValue + FMath::InterpStep(0.5, OutValue) * OutValue;


	return t < 0.5 ? 4 * t * t * t : 1 - FMath::Pow(-2 * t + 2, 3) / 2;


}


FVector MathHelperFunctions::BezierCurvePosition(FVector P0, FVector P1, FVector P2, float t)
{
	float invT = 1 - t;

	return FMath::Pow(invT,2) * P0 + 2 * (invT) * t * P1 + FMath::Pow(t,2) * P2;
}






MathHelperFunctions::~MathHelperFunctions()
{
}
