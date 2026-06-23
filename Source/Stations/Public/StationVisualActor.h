// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "StationVisualActor.generated.h"

/**
 * Base class for a station's visual representation.
 *
 * Spawned and swapped at runtime by AStationActor's VisualActor component
 * whenever the assigned UStationAsset changes.
 * Subclass in Blueprint to compose any combination of meshes, Niagara, etc.
 */
UCLASS(Abstract, Blueprintable)
class STATIONS_API AStationVisualActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:

	/**
	 * Get the ItemHolder attach parent and socket
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Stations")
	USceneComponent* GetItemAnchor(FName& OutSocketName) const;
	
protected:
	
	void Interact_Implementation(AActor* InInstigator) override;
};
