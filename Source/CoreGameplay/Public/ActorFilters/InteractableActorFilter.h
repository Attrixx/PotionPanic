// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorFilters/ActorFilter.h"
#include "InteractableActorFilter.generated.h"

/** Matches actors that implement IInteractable and currently allow interaction (CanInteract) from Instigator. */
UCLASS()
class COREGAMEPLAY_API UInteractableActorFilter : public UActorFilter
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Instigator;

protected:

	bool Matches_Implementation(AActor* Candidate) const override;
};
