// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LevelGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API ALevelGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	
	void OnPostLogin(AController* NewPlayer) override;
	
protected:
	
	UPROPERTY(EditAnywhere)
	FName ViewTargetTag = FName("ViewTarget");
	
private:
	
	AActor* GetViewTarget();
	TWeakObjectPtr<AActor> CachedViewTarget;
};
