// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemAsset.generated.h"

class AItemVisualActor;

/**
 * 
 */
UCLASS()
class ITEMS_API UItemAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

protected:
	
#if WITH_EDITOR
	EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item"))
	FGameplayTagContainer ItemTags;

	/** Radius of the capsule AItemActor uses as its collision and physics body. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor", meta = (ClampMin = 0))
	float BodyRadius = 20.f;

	/** Half height of that capsule, measured from its centre. Clamped up to the radius. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor", meta = (ClampMin = 0))
	float BodyHalfHeight = 20.f;

	/** Can a player throw this item away, or must it be handed over to a holder? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	bool bCanBeThrown = true;

	/**
	 * How the item looks and sounds. Everything cosmetic -- meshes, Niagara, audio -- is composed
	 * in this Blueprint; the asset itself carries no mesh, system or sound.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TSubclassOf<AItemVisualActor> VisualActorClass;
};
