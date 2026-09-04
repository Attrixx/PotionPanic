// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "ActivityHolderTarget.generated.h"

struct FActivityExecutionState;
class UHolderComponent;
class AItemActor;

/**
 * Which holder a conclusion acts upon: the station running the activity, the instigator that
 * triggered it, or both in a given priority order. The two "First" modes fall back on the other
 * holder when the preferred one cannot serve -- what "serve" means is up to the conclusion: a free
 * holder to fill for Create, a holder carrying an item for Morph and Change Tags.
 */
UENUM(BlueprintType)
enum class EActivityHolderTarget : uint8
{
	/**
	 * Puts the item back where it was taken from: InstigatorFirst when the activity started by
	 * taking it out of the instigator's hands, StationFirst when it was already on the station.
	 */
	Origin = 0,

	/** The station's holder only. */
	Station,

	/** The instigator's holder only. A no-op when no instigator triggered the activity. */
	Instigator,

	/** The station's holder, then the instigator's. */
	StationFirst,

	/** The instigator's holder, then the station's. */
	InstigatorFirst,
};

/**
 * The holders a conclusion may act upon, ordered by priority and stripped of the ones that are not
 * there (no instigator, instigator without a holder). Build one at the top of Conclude and query
 * it; it holds raw pointers and is only meant to live for the duration of that call.
 *
 * Two rules are applied while ordering, both driven by the execution state rather than by the
 * authored target: Origin resolves against where the item came from, and any target that names the
 * instigator degrades to StationFirst once a different instigator has taken the activity over.
 */
struct ACTIVITIES_API FActivityTargetHolders
{
	FActivityTargetHolders(const FActivityExecutionState& State, EActivityHolderTarget Target);

	/** @return The highest-priority holder carrying nothing, or null when they are all full. */
	UHolderComponent* FindFreeHolder() const;

	/** @return The item carried by the highest-priority holder carrying one, or null when none does. */
	AItemActor* FindCarriedItem() const;

	/** @return The highest-priority holder, whether or not it can serve. Null only if IsEmpty(). */
	UHolderComponent* GetPreferred() const { return Holders.IsEmpty() ? nullptr : Holders[0]; }

	bool IsEmpty() const { return Holders.IsEmpty(); }

private:

	TArray<UHolderComponent*, TInlineAllocator<2>> Holders;
};
