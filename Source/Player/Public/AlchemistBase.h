// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AlchemistBase.generated.h"

UENUM(BlueprintType)
enum class EIntentType : uint8
{
	PickUpOrDrop,
	Throw,
	Interact,
	Dash
};

UCLASS()
class PLAYER_API AAlchemistBase : public ACharacter
{
	GENERATED_BODY()

	AAlchemistBase();

public:

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SendIntent(EIntentType Intent);

private:

	void PickupOrDrop();
	void Throw();
	void Interact();
	void Dash();
};
