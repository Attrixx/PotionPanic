// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemAsset.h"
#include "IngredientTypes.h"
#include "IngredientData.generated.h"

/**
 * DataAsset identifying an ingredient.
 * Contains metadata about retrieval difficulty and type, used by Dispenser Stations.
 */
UCLASS()
class ITEMS_API UIngredientData : public UItemAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient")
    EIngredientType Type = EIngredientType::Raw;

    // Metadata: How this ingredient is typically retrieved.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Retrieval")
    ERetrievalType RetrievalType = ERetrievalType::Pickup;

    // Configuration for QTE retrieval (Shake, Pluck, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Retrieval", meta = (EditCondition = "RetrievalType == ERetrievalType::QTE || RetrievalType == ERetrievalType::Interaction"))
    FQTEData QTEConfig;

    // Configuration for Destructible retrieval (Break, Spell)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Retrieval", meta = (EditCondition = "RetrievalType == ERetrievalType::Destructible"))
    FDestructionData DestructionConfig;
};