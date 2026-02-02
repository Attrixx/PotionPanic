// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "LobbySpawnPoint.generated.h"


UCLASS()
class COREGAMEPLAY_API ALobbySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	ALobbySpawnPoint();
public : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite);
	bool bIsOccupied = false;

	
};
