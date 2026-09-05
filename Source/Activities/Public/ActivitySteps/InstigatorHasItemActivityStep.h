// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include "GameplayTagContainer.h"
#include "InstigatorHasItemActivityStep.generated.h"

class AItemActor;

/** How AllowedTags is confronted with the tags of the item the instigator carries. */
UENUM()
enum class EItemTagMatchMethod : uint8
{
	/** The item must carry every AllowedTags entry. An empty AllowedTags therefore accepts any item. */
	HasAll,

	/** The item must carry at least one AllowedTags entry, which must not be empty. */
	HasAny,

	/** AllowedTags is ignored: any item passes, as long as BlockedTags keeps it out. */
	AnyItem,
};

/** What the step does with a check that does not pass. */
UENUM()
enum class EInstigatorItemFailurePolicy : uint8
{
	/** A failed check on StartStep fails the step at once, so OnInteract is never reached. */
	FailOnStart,

	/** StartStep waits, but interacting with the wrong item -- or none -- fails the step. */
	WaitThenFailOnInteract,

	/** Never fails: the step ends only once the check passes, however many interacts that takes. */
	NeverFail,
};

UCLASS(DisplayName = "Instigator Has Item")
class ACTIVITIES_API UInstigatorHasItemActivitySettings : public UActivityStepSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UActivityStep* CreateStep_Implementation(UObject* Outer) const override;

public:

	UPROPERTY(EditAnywhere, Category="")
	EItemTagMatchMethod MatchMethod = EItemTagMatchMethod::HasAll;

	UPROPERTY(EditAnywhere, Category="")
	EInstigatorItemFailurePolicy FailurePolicy = EInstigatorItemFailurePolicy::FailOnStart;

	/** Tags the carried item must have, read through MatchMethod. */
	UPROPERTY(EditAnywhere, Category="", meta=(EditCondition="MatchMethod != EItemTagMatchMethod::AnyItem", Categories="Item"))
	FGameplayTagContainer AllowedTags;

	/** Tags that disqualify the carried item. A single one present is enough to fail the check. */
	UPROPERTY(EditAnywhere, Category="", meta=(Categories="Item"))
	FGameplayTagContainer BlockedTags;
};

/**
 * Gate step: passes only while the instigator is valid and carries an item whose tags satisfy
 * AllowedTags/BlockedTags. Re-checked on every interact, so a step left waiting can still be
 * satisfied by coming back with the right item.
 */
UCLASS()
class ACTIVITIES_API UInstigatorHasItemActivityStep : public UActivityStep
{
	GENERATED_BODY()

	void StartStep_Implementation(AActor* LastInstigator) override;
	void OnInteract_Implementation(AActor* Instigator) override;

	/** @return The item Instigator carries, or null if there is no instigator, no holder or no item. */
	const AItemActor* FindCarriedItem(const AActor* Instigator) const;

	/** @return Whether Item satisfies AllowedTags and BlockedTags. Item may be null, which never passes. */
	bool MatchesTags(const AItemActor* Item) const;

	friend UInstigatorHasItemActivitySettings;

	EItemTagMatchMethod MatchMethod = EItemTagMatchMethod::HasAll;
	EInstigatorItemFailurePolicy FailurePolicy = EInstigatorItemFailurePolicy::FailOnStart;
	FGameplayTagContainer AllowedTags;
	FGameplayTagContainer BlockedTags;
};
