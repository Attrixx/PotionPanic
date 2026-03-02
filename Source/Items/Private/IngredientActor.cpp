// Fill out your copyright notice in the Description page of Project Settings.

#include "IngredientActor.h"
#include "IngredientData.h"

DEFINE_LOG_CATEGORY_STATIC(LogIngredient, Log, All);

AIngredientActor::AIngredientActor()
{
}

void AIngredientActor::SetItemAsset(UItemAsset &NewItemAsset)
{
    // Validate that the asset is of the correct type
    if (!NewItemAsset.IsA<UIngredientData>())
    {
        UE_LOG(LogIngredient, Error, TEXT("AIngredientActor::SetItemAsset called with invalid asset type: %s. Expected UIngredientData."), *NewItemAsset.GetName());
        return;
    }

    Super::SetItemAsset(NewItemAsset);
}

void AIngredientActor::BeginPlay()
{
    Super::BeginPlay();

    // Ensure we have valid data on BeginPlay
    if (GetItemAsset() && !GetItemAsset()->IsA<UIngredientData>())
    {
        UE_LOG(LogIngredient, Error, TEXT("%s initialized with non-IngredientData asset!"), *GetName());
    }
}

const UIngredientData *AIngredientActor::GetIngredientData() const
{
    return Cast<UIngredientData>(GetItemAsset());
}

EIngredientType AIngredientActor::GetIngredientType() const
{
    if (const UIngredientData *Data = GetIngredientData())
    {
        return Data->Type;
    }
    return EIngredientType::Raw; // Default fallback
}

void AIngredientActor::NotifyHit(UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    if (const UIngredientData *Data = GetIngredientData())
    {
        if (Data->DestructionConfig.bIsFragile)
        {
            // Check if the collision force exceeds the threshold
            // Note: NormalImpulse.Size() gives the magnitude of the impulse
            if (NormalImpulse.Size() > Data->DestructionConfig.ImpactThreshold)
            {
                UE_LOG(LogIngredient, Display, TEXT("Fragile Ingredient %s broke! Impulse: %f > Threshold: %f"), *GetName(), NormalImpulse.Size(), Data->DestructionConfig.ImpactThreshold);
                // TODO (Nath): Spawn broken mesh, play sound, destroy actor
                Destroy();
            }
        }
    }
}
