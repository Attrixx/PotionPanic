// Fill out your copyright notice in the Description page of Project Settings.

#include "IngredientActor.h"
#include "IngredientData.h"
#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogIngredient, Log, All);

AIngredientActor::AIngredientActor()
{
}

void AIngredientActor::SetItemAsset(UItemAsset* NewItemAsset)
{
	if (!NewItemAsset)
	{
		return;
	}

	if (!NewItemAsset->IsA<UIngredientData>())
	{
		UE_LOG(LogIngredient, Error, TEXT("AIngredientActor::SetItemAsset called with invalid asset type: %s. Expected UIngredientData."), *NewItemAsset->GetName());
		return;
	}

	Super::SetItemAsset(NewItemAsset);
}

void AIngredientActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetItemAsset() && !GetItemAsset()->IsA<UIngredientData>())
	{
		UE_LOG(LogIngredient, Error, TEXT("%s initialized with non-IngredientData asset!"), *GetName());
	}
}

void AIngredientActor::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (IsActorBeingDestroyed() || !IsBreakable())
	{
		return;
	}

	// Fallback for thrown items when physics impulse is not reported reliably.
	const bool bHitStaticWorld = (Other == nullptr) || (OtherComp && OtherComp->Mobility == EComponentMobility::Static);
	if (!bHitStaticWorld)
	{
		return;
	}

	const float BreakThreshold = GetBreakImpulseThreshold();
	const float ImpactImpulse = NormalImpulse.Size();
	if (ImpactImpulse >= BreakThreshold)
	{
		DestroyItem(true);
		return;
	}

	const FVector ImpactVelocity = MyComp ? MyComp->GetComponentVelocity() : GetVelocity();
	const float ImpactSpeed = ImpactVelocity.Size();
	// TODO (Nath): Move fallback speed threshold to ingredient/item data for per-item tuning (e.g. fragile vial).
	const float FallbackSpeedThreshold = FMath::Max(600.0f, BreakThreshold);
	if (ImpactSpeed >= FallbackSpeedThreshold)
	{
		DestroyItem(true);
	}
}

const UIngredientData* AIngredientActor::GetIngredientData() const
{
	return Cast<UIngredientData>(GetItemAsset());
}

EIngredientType AIngredientActor::GetIngredientType() const
{
	if (const UIngredientData* Data = GetIngredientData())
	{
		return Data->Type;
	}

	return EIngredientType::Raw;
}

FIngredientStateDescriptor AIngredientActor::GetIngredientStateDescriptor() const
{
	if (const UIngredientData* Data = GetIngredientData())
	{
		return Data->StateDescriptor;
	}

	return FIngredientStateDescriptor{};
}
