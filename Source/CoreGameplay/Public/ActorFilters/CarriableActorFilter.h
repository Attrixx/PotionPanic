// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorFilters/ActorFilter.h"
#include "CarriableActorFilter.generated.h"

/**
 * Matches an actor that can be picked up: one that implements UCarriable itself (a loose item
 * lying free), or one carrying an occupied UHolderComponent (a station, or another holder,
 * offering its item up). Both are folded into a single filter so the range component's priority
 * ranking picks the best candidate across the two, rather than one kind always winning over the
 * other regardless of score.
 */
UCLASS()
class COREGAMEPLAY_API UCarriableActorFilter : public UActorFilter
{
	GENERATED_BODY()

public:

	/** Never matched, so a holder is not offered its own Carriable back. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Ignored;

protected:

	bool Matches_Implementation(AActor* Candidate) const override;
};
