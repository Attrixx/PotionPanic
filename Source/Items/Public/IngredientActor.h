// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemActor.h"
#include "IngredientActor.generated.h"

class UIngredientData;

/**
 * Physical representation of an ingredient in the world.
 */
UCLASS()
class ITEMS_API AIngredientActor : public AItemActor
{
    GENERATED_BODY()

public:
    AIngredientActor();

    virtual void SetItemAsset(UItemAsset &NewItemAsset) override;

    UFUNCTION(BlueprintPure, Category = "Ingredient")
    const UIngredientData *GetIngredientData() const;

    UFUNCTION(BlueprintPure, Category = "Ingredient")
    EIngredientType GetIngredientType() const;

    // TODO (Nath): Add 'OnProcessed' event/delegate when an ingredient changes from Raw -> Processed.

protected:
    virtual void BeginPlay() override;

    virtual void NotifyHit(UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit) override;

    // TODO (Nath): Implement specific physical behaviors (e.g. Ice melting if near heat source).
};