// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "LobbyLevelScriptActor.generated.h"

DEFINE_LOG_CATEGORY_STATIC(MS_LobbyLevelScriptActor, Log, All);

class ATriggerBox;

/**
 * 
 */
UCLASS()
class LOBBY_API ALobbyLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()

public:

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void RegisterTriggerBoxes(const TMap<ECameraPosition, ATriggerBox*>& TriggerBoxes);

	UFUNCTION(BlueprintCallable)
	void ToggleShowPlayerPreviews(bool bShow = true);

	UFUNCTION(BlueprintCallable)
	void OnStartupSequenceFinished();

	bool IsActorInTriggerBox(AActor* Actor, ECameraPosition TriggerBoxType) const;

private:

	UFUNCTION()
	void OnTriggerBoxBeginOverlap(AActor* TriggerBox, AActor* OtherActor);
	UFUNCTION()
	void OnTriggerBoxEndOverlap(AActor* TriggerBox, AActor* OtherActor);
	bool IsAnyActorInTriggerBox(TSubclassOf<AActor> ClassToSearch, ECameraPosition TriggerBoxType) const;

private:

	TMap<ECameraPosition, ATriggerBox*> RegisteredTriggerBoxes;
	
};
