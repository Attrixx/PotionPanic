// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemAsset.h"
#include "IngredientTypes.h"
#include "IngredientData.generated.h"

/**
 * Data-only ingredient asset.
 * Describes ingredient category and transformation flags, without gameplay logic.
 */
UCLASS()
class ITEMS_API UIngredientData : public UItemAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient")
	EIngredientType Type = EIngredientType::Raw;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient")
	FIngredientStateDescriptor StateDescriptor;

	// TODO (Nath): Keep ingredient-related rules in station/interaction systems, not in Items.
};
