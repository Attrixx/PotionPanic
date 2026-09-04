// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ItemVisualActor.generated.h"

class AItemActor;

/**
 * Base class for an item's visual representation.
 *
 * Spawned and swapped at runtime by AItemActor's VisualActor component whenever the assigned
 * UItemAsset changes. Subclass in Blueprint to compose any combination of meshes, Niagara and
 * audio, and to react to the gameplay events AItemActor relays here.
 *
 * Collision, physics and breakability stay on AItemActor, and its capsule is sized by the item
 * asset: this actor only draws the item and plays its effects.
 */
UCLASS(Abstract, Blueprintable)
class ITEMS_API AItemVisualActor : public AActor
{
	GENERATED_BODY()

public:

	AItemVisualActor();

	/** Thrown away by a holder, with the velocity it was given. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Items")
	void OnItemThrown(const FVector& Velocity);

	/**
	 * Hit hard enough to break. The item is destroyed as soon as this returns, and this actor with
	 * it, so whatever is spawned here must stand on its own: an emitter or a sound placed in the
	 * world, never a component attached to this actor.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Items")
	void OnItemBroken();

	/**
	 * The item's tags changed: a new asset was applied, or an activity edited them along the way.
	 * Also fires once when the item first receives its asset.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Items")
	void OnItemTagsChanged(const FGameplayTagContainer& NewTags);

	/** Called by AItemActor as soon as it spawns this visual. */
	void SetItemActor(AItemActor* InItemActor) { ItemActor = InItemActor; }

protected:

	/** The item this visual belongs to. Set before any of the events above can fire. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Items")
	TObjectPtr<AItemActor> ItemActor;
};
