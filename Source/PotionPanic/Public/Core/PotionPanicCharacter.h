// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PotionPanicCharacter.generated.h"

UCLASS()
class POTIONPANIC_API APotionPanicCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APotionPanicCharacter();

protected:
	void BeginPlay() override;

public:

	/*
	* Input related methods
	*/
	void OnInteract();
	void OnCarry();

private:
	TObjectPtr<class UCamTargetComponent> CamTargetComponent = nullptr;
};
