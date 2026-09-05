// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivitySteps/InstigatorHasItemActivityStep.h"
#include "ActivityStepResult.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include <Misc/DataValidation.h>

DEFINE_LOG_CATEGORY_STATIC(MS_InstigatorHasItemActivityStep, Verbose, All);

#if WITH_EDITOR
EDataValidationResult UInstigatorHasItemActivitySettings::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	switch (MatchMethod)
	{
	case EItemTagMatchMethod::HasAll:
		// Hierarchical matching, so Allowed 'Item.Fiole.Vide' against Blocked 'Item.Fiole' counts as
		// an overlap, while the other way around does not.
		if (AllowedTags.HasAny(BlockedTags))
		{
			Context.AddError(FText::FromString("AllowedTags and BlockedTags overlap: no item can ever pass this step."));
			Result = EDataValidationResult::Invalid;
		}
		break;

	case EItemTagMatchMethod::HasAny:
		if (AllowedTags.IsEmpty())
		{
			Context.AddError(FText::FromString("AllowedTags is empty with the HasAny method: no item can ever pass this step."));
			Result = EDataValidationResult::Invalid;
		}
		break;

	case EItemTagMatchMethod::AnyItem:
		if (!AllowedTags.IsEmpty())
		{
			Context.AddWarning(FText::FromString("AllowedTags is ignored by the AnyItem method."));
		}
		break;
	}

	return Result;
}
#endif

UActivityStep* UInstigatorHasItemActivitySettings::CreateStep_Implementation(UObject* Outer) const
{
	check(MatchMethod != EItemTagMatchMethod::HasAny || !AllowedTags.IsEmpty());

	auto* Step = NewObject<UInstigatorHasItemActivityStep>(Outer);
	Step->MatchMethod = MatchMethod;
	Step->FailurePolicy = FailurePolicy;
	Step->AllowedTags = AllowedTags;
	Step->BlockedTags = BlockedTags;
	return Step;
}

void UInstigatorHasItemActivityStep::StartStep_Implementation(AActor* LastInstigator)
{
	const AItemActor* Item = FindCarriedItem(LastInstigator);
	if (MatchesTags(Item))
	{
		UE_LOGFMT(MS_InstigatorHasItemActivityStep, Verbose, "'{0}' starts holding a matching '{1}'.",
			GetNameSafe(LastInstigator), GetNameSafe(Item));

		FinishStep(FActivityStepResult{
			.Status = EActivityStepStatus::Success,
			.Score = 0,
		});
		return;
	}

	if (FailurePolicy == EInstigatorItemFailurePolicy::FailOnStart)
	{
		UE_LOGFMT(MS_InstigatorHasItemActivityStep, Verbose, "'{0}' does not hold a matching item on start.",
			GetNameSafe(LastInstigator));

		FinishStep(FActivityStepResult{
			.Status = EActivityStepStatus::Fail,
			.Score = 0,
		});
		return;
	}

	// Waiting: an interact may bring the right item later.
	UE_LOGFMT(MS_InstigatorHasItemActivityStep, Verbose, "Waiting for an instigator holding a matching item.");
}

void UInstigatorHasItemActivityStep::OnInteract_Implementation(AActor* Instigator)
{
	const AItemActor* Item = FindCarriedItem(Instigator);
	if (MatchesTags(Item))
	{
		UE_LOGFMT(MS_InstigatorHasItemActivityStep, Verbose, "'{0}' interacts holding a matching '{1}'.",
			GetNameSafe(Instigator), GetNameSafe(Item));

		FinishStep(FActivityStepResult{
			.Status = EActivityStepStatus::Success,
			.Score = 0,
		});
		return;
	}

	if (FailurePolicy == EInstigatorItemFailurePolicy::WaitThenFailOnInteract)
	{
		UE_LOGFMT(MS_InstigatorHasItemActivityStep, Verbose, "'{0}' interacts without a matching item.",
			GetNameSafe(Instigator));

		FinishStep(FActivityStepResult{
			.Status = EActivityStepStatus::Fail,
			.Score = 0,
		});
	}
}

const AItemActor* UInstigatorHasItemActivityStep::FindCarriedItem(const AActor* Instigator) const
{
	if (!IsValid(Instigator))
		return nullptr;

	const UHolderComponent* Holder = Instigator->FindComponentByClass<UHolderComponent>();
	if (!Holder)
		return nullptr;

	return Cast<AItemActor>(Holder->GetCarriable());
}

bool UInstigatorHasItemActivityStep::MatchesTags(const AItemActor* Item) const
{
	// Empty hands never pass: there are no tags to read.
	if (!Item)
		return false;

	const FGameplayTagContainer& ItemTags = Item->GetItemTags();

	// HasAny on an empty container is false, so an empty BlockedTags blocks nothing.
	if (ItemTags.HasAny(BlockedTags))
		return false;

	switch (MatchMethod)
	{
	case EItemTagMatchMethod::HasAll:
		return ItemTags.HasAll(AllowedTags);

	case EItemTagMatchMethod::HasAny:
		return ItemTags.HasAny(AllowedTags);

	case EItemTagMatchMethod::AnyItem:
		return true;

	default:
		checkNoEntry();
		return false;
	}
}
