// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "StationsLayoutSubsystem.generated.h"

class AStationActor;
class UStationAsset;
class UStationsLayoutLayer;

/** A station tracked by the layout subsystem, along with the asset it was placed with. */
USTRUCT()
struct FTrackedStation
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AStationActor> Station;

	/** Asset the station held before any layer was applied. Restored by ResetToDefaultLayout. */
	UPROPERTY()
	TObjectPtr<UStationAsset> DefaultAsset;
};

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

	/** Restores every tracked station to the asset it was placed with, discarding all applied layers. */
	UFUNCTION(BlueprintCallable)
	void ResetToDefaultLayout();
	
private:
	
	void GetStationsFromWorld(UWorld* InWorld);
	
	UPROPERTY()
	TArray<FTrackedStation> Stations;
};
