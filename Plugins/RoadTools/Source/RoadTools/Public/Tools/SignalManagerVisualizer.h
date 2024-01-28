// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "GameFramework/Actor.h"

/**
 * 
 */
class FSignalManagerVisualizer : public FComponentVisualizer
{
public:

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView*, FPrimitiveDrawInterface* PDI) override;

};
