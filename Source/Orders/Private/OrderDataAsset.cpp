#include "OrderDataAsset.h"
#include "Engine/AssetManager.h"
#include "ItemAsset.h"

namespace
{
UItemAsset* ResolveItemAsset(const FPrimaryAssetId& ItemId)
{
	if (!ItemId.IsValid())
	{
		return nullptr;
	}

	if (UItemAsset* Loaded = UAssetManager::Get().GetPrimaryAssetObject<UItemAsset>(ItemId))
	{
		return Loaded;
	}

	const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
	return Cast<UItemAsset>(AssetPath.TryLoad());
}

bool HasAnyAcceptedOutput(const UOrderDataAsset& DataAsset)
{
	if (DataAsset.RequiredOutputItemId.IsValid())
	{
		return true;
	}

	for (const FPrimaryAssetId& ItemId : DataAsset.AdditionalAcceptedOutputItems)
	{
		if (ItemId.IsValid())
		{
			return true;
		}
	}

	return false;
}
}

bool UOrderDataAsset::IsOrderDefinitionValid(FText& OutFailureReason) const
{
	OutFailureReason = FText::GetEmpty();

	if (!HasAnyAcceptedOutput(*this) && RequiredOutputDataTags.Num() == 0)
	{
		OutFailureReason = FText::FromString(TEXT("Order requires at least one accepted output item id or required data tags."));
		return false;
	}

	if (TimeLimitSeconds <= 0.0f)
	{
		OutFailureReason = FText::FromString(TEXT("TimeLimitSeconds must be > 0."));
		return false;
	}

	if (BaseScore < 0 || TimeBonusMax < 0 || WrongDeliveryPenalty < 0 || ExpirePenalty < 0)
	{
		OutFailureReason = FText::FromString(TEXT("Score and penalty fields must be >= 0."));
		return false;
	}

	return true;
}

bool UOrderDataAsset::MatchesDeliveredItem(const FPrimaryAssetId& DeliveredItemId, FText& OutFailureReason) const
{
	OutFailureReason = FText::GetEmpty();

	if (!DeliveredItemId.IsValid())
	{
		OutFailureReason = FText::FromString(TEXT("Delivered item id is invalid."));
		return false;
	}

	bool bAcceptedById = false;
	if (RequiredOutputItemId.IsValid())
	{
		bAcceptedById = RequiredOutputItemId == DeliveredItemId;
	}

	if (!bAcceptedById)
	{
		for (const FPrimaryAssetId& AcceptedId : AdditionalAcceptedOutputItems)
		{
			if (AcceptedId.IsValid() && AcceptedId == DeliveredItemId)
			{
				bAcceptedById = true;
				break;
			}
		}
	}

	const bool bHasIdConstraints = HasAnyAcceptedOutput(*this);
	if (bHasIdConstraints && !bAcceptedById)
	{
		OutFailureReason = FText::FromString(TEXT("Delivered item does not match accepted output item ids."));
		return false;
	}

	if (RequiredOutputDataTags.Num() == 0)
	{
		return true;
	}

	const UItemAsset* ItemAsset = ResolveItemAsset(DeliveredItemId);
	if (ItemAsset == nullptr)
	{
		OutFailureReason = FText::FromString(TEXT("Unable to resolve delivered item asset for tag validation."));
		return false;
	}

	int32 MatchedTagCount = 0;
	for (const FName RequiredTag : RequiredOutputDataTags)
	{
		if (RequiredTag.IsNone())
		{
			continue;
		}

		if (ItemAsset->DataTags.Contains(RequiredTag))
		{
			++MatchedTagCount;
		}
	}

	const int32 RequiredTagCount = RequiredOutputDataTags.FilterByPredicate([](const FName Tag) { return !Tag.IsNone(); }).Num();
	if (RequiredTagCount == 0)
	{
		return true;
	}

	const bool bTagMatch = bRequireAllOutputDataTags
		? MatchedTagCount >= RequiredTagCount
		: MatchedTagCount > 0;

	if (!bTagMatch)
	{
		OutFailureReason = FText::FromString(TEXT("Delivered item does not satisfy required output data tags."));
		return false;
	}

	return true;
}

#if WITH_EDITOR
EDataValidationResult UOrderDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	Super::IsDataValid(Context);

	FText FailureReason;
	if (!IsOrderDefinitionValid(FailureReason))
	{
		Context.AddError(FailureReason);
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif
