// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorFilter.generated.h"

/**
 * Polymorphic predicate over an AActor.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class COREGAMEPLAY_API UActorFilter : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	bool Matches(AActor* Candidate) const;

protected:

	virtual bool Matches_Implementation(AActor* Candidate) const PURE_VIRTUAL(UActorFilter::Matches, return false;)
};
