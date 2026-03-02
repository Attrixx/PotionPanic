#include "Cauldron.h"
#include "CarriableComponent.h"
#include "Engine/AssetManager.h"
#include "IngredientData.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include "Algo/Sort.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HolderComponent.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(MS_Cauldron, Log, All);

namespace
{
UItemAsset* ResolveItemAssetFromId(const FPrimaryAssetId& ItemId)
{
	if (!ItemId.IsValid())
	{
		return nullptr;
	}

	if (UItemAsset* LoadedAsset = UAssetManager::Get().GetPrimaryAssetObject<UItemAsset>(ItemId))
	{
		return LoadedAsset;
	}

	const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
	return Cast<UItemAsset>(AssetPath.TryLoad());
}

bool IsIngredientAssetId(const FPrimaryAssetId& ItemId)
{
	const UItemAsset* ItemAsset = ResolveItemAssetFromId(ItemId);
	return Cast<UIngredientData>(ItemAsset) != nullptr;
}
}

ACauldron::ACauldron()
{
	bReplicates = true;
}

void ACauldron::Interact(APlayerController& InInstigator)
{
	if (!HasAuthority())
	{
		return;
	}

	APawn* InstigatorPawn = InInstigator.GetPawn();
	if (InstigatorPawn == nullptr)
	{
		return;
	}

	UHolderComponent* PlayerHolder = InstigatorPawn->FindComponentByClass<UHolderComponent>();
	if (PlayerHolder == nullptr || Carriable == nullptr)
	{
		return;
	}

	UCarriableComponent* PlayerCarriable = PlayerHolder->GetCarriable();
	if (PlayerCarriable && PlayerCarriable != Carriable)
	{
		AItemActor* HeldItemActor = Cast<AItemActor>(PlayerCarriable->GetOwner());
		if (!HeldItemActor)
		{
			return;
		}

		const FPrimaryAssetId HeldItemId = HeldItemActor->GetItemAssetId();
		if (!CanAcceptIngredientAssetId(HeldItemId))
		{
			return;
		}

		if (AddIngredientAssetId(HeldItemId))
		{
			PlayerHolder->Replace(nullptr);
			HeldItemActor->DestroyItem(true);
		}
		return;
	}

	if (PlayerCarriable == Carriable)
	{
		// Drop cauldron from player's hands.
		PlayerHolder->Replace(nullptr);
		return;
	}

	if (UHolderComponent* CurrentHolder = Carriable->GetHolder())
	{
		CurrentHolder->Replace(nullptr);
	}

	PlayerHolder->Replace(Carriable);
}

void ACauldron::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACauldron, IngredientContents);
	DOREPLIFETIME(ACauldron, FillRatio);
	DOREPLIFETIME(ACauldron, LiquidTint);
	DOREPLIFETIME(ACauldron, VisualFlags);
}

bool ACauldron::AddIngredientAssetId(FPrimaryAssetId IngredientAssetId)
{
	if (!HasAuthority() || !CanAcceptIngredientAssetId(IngredientAssetId))
	{
		return false;
	}

	IngredientContents.Add(IngredientAssetId);
	UpdateFillRatioFromContents();

	OnContentsChangedBP();
	OnVisualStateChangedBP();
	return true;
}

bool ACauldron::AddContentAssetId(FPrimaryAssetId ContentAssetId)
{
	if (!HasAuthority() || !CanAcceptContentAssetId(ContentAssetId))
	{
		return false;
	}

	IngredientContents.Add(ContentAssetId);
	UpdateFillRatioFromContents();
	OnContentsChangedBP();
	OnVisualStateChangedBP();
	return true;
}

bool ACauldron::CanAcceptContentAssetId(FPrimaryAssetId ContentAssetId) const
{
	if (!ContentAssetId.IsValid())
	{
		return false;
	}

	return IngredientContents.Num() < FMath::Max(1, MaxIngredientCount);
}

bool ACauldron::CanAcceptIngredientAssetId(FPrimaryAssetId IngredientAssetId) const
{
	if (!CanAcceptContentAssetId(IngredientAssetId))
	{
		return false;
	}

	if (!bAcceptOnlyIngredientAssets)
	{
		return true;
	}

	return IsIngredientAssetId(IngredientAssetId);
}

bool ACauldron::RemoveIngredientAssetId(FPrimaryAssetId IngredientAssetId)
{
	if (!HasAuthority() || !IngredientAssetId.IsValid())
	{
		return false;
	}

	const int32 Index = IngredientContents.IndexOfByKey(IngredientAssetId);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	IngredientContents.RemoveAt(Index);
	UpdateFillRatioFromContents();
	OnContentsChangedBP();
	OnVisualStateChangedBP();
	return true;
}

bool ACauldron::ConsumeIngredientAssetIds(const TArray<FPrimaryAssetId>& IngredientAssetIds, bool bRequireExactCounts)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (IngredientAssetIds.Num() == 0)
	{
		return true;
	}

	TMap<FPrimaryAssetId, int32> RequiredCounts;
	for (const FPrimaryAssetId& IngredientAssetId : IngredientAssetIds)
	{
		if (!IngredientAssetId.IsValid())
		{
			if (bRequireExactCounts)
			{
				return false;
			}
			continue;
		}

		RequiredCounts.FindOrAdd(IngredientAssetId)++;
	}

	if (RequiredCounts.Num() == 0)
	{
		return false;
	}

	TMap<FPrimaryAssetId, int32> AvailableCounts;
	for (const FPrimaryAssetId& IngredientAssetId : IngredientContents)
	{
		AvailableCounts.FindOrAdd(IngredientAssetId)++;
	}

	TMap<FPrimaryAssetId, int32> CountsToConsume;
	for (const TPair<FPrimaryAssetId, int32>& RequiredPair : RequiredCounts)
	{
		const int32 AvailableCount = AvailableCounts.FindRef(RequiredPair.Key);
		if (bRequireExactCounts && AvailableCount < RequiredPair.Value)
		{
			return false;
		}

		const int32 ConsumeCount = bRequireExactCounts
			? RequiredPair.Value
			: FMath::Min(AvailableCount, RequiredPair.Value);
		if (ConsumeCount > 0)
		{
			CountsToConsume.Add(RequiredPair.Key, ConsumeCount);
		}
	}

	if (CountsToConsume.Num() == 0)
	{
		return false;
	}

	for (int32 Index = IngredientContents.Num() - 1; Index >= 0; --Index)
	{
		const FPrimaryAssetId& IngredientAssetId = IngredientContents[Index];
		int32* RemainingToConsume = CountsToConsume.Find(IngredientAssetId);
		if (RemainingToConsume == nullptr || *RemainingToConsume <= 0)
		{
			continue;
		}

		IngredientContents.RemoveAt(Index);
		--(*RemainingToConsume);
	}

	UpdateFillRatioFromContents();
	OnContentsChangedBP();
	OnVisualStateChangedBP();
	return true;
}

void ACauldron::ClearIngredients()
{
	if (!HasAuthority())
	{
		return;
	}

	IngredientContents.Reset();
	UpdateFillRatioFromContents();
	OnContentsChangedBP();
	OnVisualStateChangedBP();
}

TArray<FPrimaryAssetId> ACauldron::GetIngredientAssetIdsSorted() const
{
	TArray<FPrimaryAssetId> SortedContents = IngredientContents;
	Algo::Sort(SortedContents, [](const FPrimaryAssetId& Left, const FPrimaryAssetId& Right)
	{
		return Left.ToString() < Right.ToString();
	});
	return SortedContents;
}

void ACauldron::UpdateFillRatioFromContents()
{
	FillRatio = FMath::Clamp(static_cast<float>(IngredientContents.Num()) / static_cast<float>(FMath::Max(1, MaxIngredientVisualCount)), 0.0f, 1.0f);
}

void ACauldron::SetFillRatio(float NewFillRatio)
{
	if (!HasAuthority())
	{
		return;
	}

	FillRatio = FMath::Clamp(NewFillRatio, 0.0f, 1.0f);
	OnVisualStateChangedBP();
}

void ACauldron::SetLiquidTint(FLinearColor NewTint)
{
	if (!HasAuthority())
	{
		return;
	}

	LiquidTint = NewTint;
	OnVisualStateChangedBP();
}

void ACauldron::SetVisualFlags(const TArray<FName>& NewFlags)
{
	if (!HasAuthority())
	{
		return;
	}

	VisualFlags = NewFlags;
	OnVisualStateChangedBP();
}

void ACauldron::OnRep_IngredientContents()
{
	OnContentsChangedBP();
}

void ACauldron::OnRep_VisualState()
{
	OnVisualStateChangedBP();
}
