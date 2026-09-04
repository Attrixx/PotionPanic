// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
};
