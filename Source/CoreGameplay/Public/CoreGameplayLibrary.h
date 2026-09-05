// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreGameplayLibrary.generated.h"

UCLASS()
class COREGAMEPLAY_API UCoreGameplayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Searches Actor, then its attach parent, then that parent's own attach parent, and so on, for
	 * the first component matching ComponentClass. Use it to reach a component owned by an actor's
	 * attach parent instead of the actor that was actually hit or overlapped, e.g. a station's
	 * visual representation is a separate child actor that carries none of the station's components.
	 */
	UFUNCTION(BlueprintCallable, Category = "Core Gameplay|Actor", meta = (DeterminesOutputType = "ComponentClass"))
	static UActorComponent* FindComponentInAttachChain(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass);

	/** Templated version of FindComponentInAttachChain that handles casting for you. */
	template<class T>
	static T* FindComponentInAttachChain(AActor* Actor)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value,
			"'T' template parameter to FindComponentInAttachChain must be derived from UActorComponent");

		return static_cast<T*>(FindComponentInAttachChain(Actor, T::StaticClass()));
	}

	/**
	 * Picks a colour for Tag: arbitrary, but the same one every run, on every machine and in every
	 * build. Use it to tell tags apart on screen -- debug draws, icon tints, editor widgets --
	 * without hand-picking a colour per tag.
	 * The result is fully opaque, and always has at least one of R, G and B at 1.
	 */
	UFUNCTION(BlueprintPure, Category = "Core Gameplay|Tags")
	static FLinearColor MakeColorFromTag(FGameplayTag Tag);

	/**
	 * Same as MakeColorFromTag, for a whole container. A container is a set, so the colour does not
	 * depend on the order its tags were added in. An empty container has its own stable colour.
	 */
	UFUNCTION(BlueprintPure, Category = "Core Gameplay|Tags")
	static FLinearColor MakeColorFromTagContainer(const FGameplayTagContainer& Tags);

private:

	/**
	 * Hashes a tag by its name rather than through GetTypeHash, which goes through the FName
	 * comparison index -- a value assigned in registration order, so it differs between runs and
	 * between machines. A colour derived from it would not be stable.
	 */
	static uint32 HashTag(const FGameplayTag& Tag);

	static FLinearColor MakeColorFromHash(uint32 Hash);
};
