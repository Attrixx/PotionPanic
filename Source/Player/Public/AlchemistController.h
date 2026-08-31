// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AlchemistController.generated.h"

/**
 *
 */
UCLASS(Abstract)
class PLAYER_API AAlchemistController : public APlayerController
{
	GENERATED_BODY()

public:

	AAlchemistController();
	void BeginPlay() override;
	void SetupInputComponent() override;

protected:

	AActor* FindViewTarget() const;

	UPROPERTY(EditAnywhere)
	FName ViewTargetTag;
};
