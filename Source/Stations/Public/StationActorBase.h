// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ActivityAsset.h"
#include "StationActorBase.generated.h"

UCLASS()
class STATIONS_API AStationActorBase : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	AStationActorBase();

	void Interact(APlayerController& InInstigator) override;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TObjectPtr<UActivityAsset>> Activities;
};
