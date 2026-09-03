// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorFilters/ActorFilter.h"
#include "FreeHolderActorFilter.generated.h"

/** Matches actors carrying a UHolderComponent with nothing on it, so able to receive a Carriable. */
UCLASS()
class COREGAMEPLAY_API UFreeHolderActorFilter : public UActorFilter
{
	GENERATED_BODY()

public:

	/** Never matched, so a holder is not offered its own Carriable back. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Ignored;

protected:

	bool Matches_Implementation(AActor* Candidate) const override;
};
