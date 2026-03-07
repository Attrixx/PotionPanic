// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AlchemyGameMode.generated.h"

class AAlchemyWorldSettings;

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class GAMEFLOW_API AAlchemyGameMode : public AGameModeBase
{
	GENERATED_BODY()

	void OnPostLogin(AController* NewPlayer) override;

protected:

	UPROPERTY(EditAnywhere)
	FName ViewTargetTag;

private:

	AActor* GetViewTarget();
	TWeakObjectPtr<AActor> CachedViewTarget;
};
