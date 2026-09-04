// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemAsset.h"
#include "ItemTags.h"
#include "ItemVisualActor.h"
#include <Misc/DataValidation.h>

#if WITH_EDITOR
EDataValidationResult UItemAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	
	if (!ItemTags.HasTag(GameTags::Item))
	{
		Context.AddError(FText::FromString("Item tags should contain at least one item tag."));
		Result = EDataValidationResult::Invalid;
	}
	
	if (!IsValid(VisualActorClass))
	{
		Context.AddError(FText::FromString("VisualActorClass is null: the item would have no body and no visual."));
		Result = EDataValidationResult::Invalid;
	}

	if (ItemTags.HasTag(GameTags::Item_None))
	{
		Context.AddError(FText::FromString(FString::Format(
			TEXT("Input tags cannot contain {0}."),
			{GameTags::Item_None.GetTag().ToString()})));
		Result = EDataValidationResult::Invalid;
	}
	
	return Result;
}
#endif
