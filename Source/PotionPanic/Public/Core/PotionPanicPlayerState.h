// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "PotionPanicPlayerState.generated.h"

class UAbilitySystemComponent;

UCLASS()
class POTIONPANIC_API APotionPanicPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	APotionPanicPlayerState();

	UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComponent;
	}

private:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
};
