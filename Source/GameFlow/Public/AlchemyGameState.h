// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AlchemyGameState.generated.h"

class UWorldData;

/**
 * 
 */
UCLASS()
class GAMEFLOW_API AAlchemyGameState : public AGameStateBase
{
	GENERATED_BODY()
	
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	
	void SetWorldData(const TSoftObjectPtr<UWorldData>& NewWorldData);
	
private:
	
	UFUNCTION()
	void OnRep_SoftWorldData();
	
	void ConfigureRecipeSystem() const;

private:

	UPROPERTY(ReplicatedUsing=OnRep_SoftWorldData)
	TSoftObjectPtr<UWorldData> SoftWorldData;
	
	UPROPERTY(Transient)
	TObjectPtr<UWorldData> WorldData;
};
