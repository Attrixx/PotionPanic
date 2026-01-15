// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PotionPanicGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API UPotionPanicGameInstance : public UGameInstance
{
	GENERATED_BODY()

public :
	void SavePlayerColor(FUniqueNetIdRepl PlayerId, FColor Color);
	FColor GetPlayerColor(FUniqueNetIdRepl);
protected : 

	TMap<FUniqueNetIdRepl, FColor> PlayerSelectedColor; 
};
