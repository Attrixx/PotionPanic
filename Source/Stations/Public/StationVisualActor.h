// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StationVisualActor.generated.h"

class AStationActor;

/**
 * Base class for a station's visual representation.
 *
 * Spawned and swapped at runtime by AStationActor's VisualActor component
 * whenever the assigned UStationAsset changes.
 * Subclass in Blueprint to compose any combination of meshes, Niagara, etc.
 */
UCLASS(Abstract, Blueprintable)
class STATIONS_API AStationVisualActor : public AActor
{
	GENERATED_BODY()
	
public:
	
	AStationVisualActor();

	/**
	 * Get the ItemHolder attach parent and socket
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Stations")
	USceneComponent* GetItemAnchor(FName& OutSocketName) const;

	/** Called by AStationActor as soon as it spawns this visual. */
	void SetStationActor(AStationActor* InStationActor) { StationActor = InStationActor; }

protected:

	/** The station this visual belongs to. Set as soon as the station spawns it. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Stations")
	TObjectPtr<AStationActor> StationActor;
};
