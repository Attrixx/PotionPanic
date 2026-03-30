// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "LobbyLevelScriptActor.generated.h"

class ATriggerBox;

/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API ALobbyLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()

public:

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void RegisterTriggerBoxes(const TMap<ETriggerBoxType, ATriggerBox*>& TriggerBoxes);

	UFUNCTION(BlueprintCallable)
	void ToggleShowPlayerPreviews(bool bShow = true);

	UFUNCTION(BlueprintCallable)
	void OnStartupSequenceFinished();

private:

	UFUNCTION()
	void OnTriggerBoxBeginOverlap(AActor* TriggerBox, AActor* OtherActor);
	UFUNCTION()
	void OnTriggerBoxEndOverlap(AActor* TriggerBox, AActor* OtherActor);

	bool IsAnyActorInTriggerBox(TSubclassOf<AActor> ClassToSearch, ETriggerBoxType TriggerBoxType) const;
	bool IsActorInTriggerBox(AActor* Actor, ETriggerBoxType TriggerBoxType) const;

private:

	TMap<ETriggerBoxType, ATriggerBox*> RegisteredTriggerBoxes;
	
};
