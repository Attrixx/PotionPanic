// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorFilters/ActorFilter.h"
#include "InterfaceActorFilter.generated.h"

/** Matches actors whose class implements the given interface. */
UCLASS()
class COREGAMEPLAY_API UInterfaceActorFilter : public UActorFilter
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UInterface> Interface;

protected:

	bool Matches_Implementation(AActor* Candidate) const override;
};
