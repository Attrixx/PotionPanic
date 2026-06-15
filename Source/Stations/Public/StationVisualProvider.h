// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StationVisualProvider.generated.h"

class AStationActor;

/**
 * 
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class STATIONS_API UStationVisualProvider : public UObject
{
	GENERATED_BODY()
	
public:
	
	// Called by StationActor::ApplyStationAsset().
	// Responsible for building any visual representation on the target actor,
	// and moving/attaching the UHolderComponent.
	// MUST be callable in editor (for layer previews). Don't do gameplay stuff here.
	virtual void Apply(AStationActor* Target) PURE_VIRTUAL(UStationVisualProvider::Apply,);
    
	virtual void Teardown(AStationActor* Target) {}
};
