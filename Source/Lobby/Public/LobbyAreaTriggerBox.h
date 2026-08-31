// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "LobbyGameState.h"
#include "LobbyAreaTriggerBox.generated.h"

/**
 * 
 */
UCLASS()
class LOBBY_API ALobbyAreaTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

public:
	ECameraPosition GetAreaType() const { return AreaType; }

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
	ECameraPosition AreaType = ECameraPosition::Exterior;
	
};
