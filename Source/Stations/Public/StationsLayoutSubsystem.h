// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "StationsLayoutSubsystem.generated.h"

class AStationActor;
class UStationsLayoutLayer;

/**
 * 
 */
UCLASS()
class STATIONS_API UStationsLayoutSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
protected:
	
	void OnWorldBeginPlay(UWorld& InWorld) override;
	
public:
	
	UFUNCTION(BlueprintCallable)
	void ApplyLayer(UStationsLayoutLayer* Layer);
	
private:
	
	void GetStationsFromWorld(UWorld* InWorld);
	
	TArray<TWeakObjectPtr<AStationActor>> Stations;
};
