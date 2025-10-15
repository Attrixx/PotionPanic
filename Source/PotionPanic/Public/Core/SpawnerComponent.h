// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpawnerComponent.generated.h"

class USocketComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class POTIONPANIC_API USpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpawnerComponent();

protected:
	void BeginPlay() override;

	TObjectPtr<USocketComponent> GetSpawnSocket(AActor* TargetActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSpawnOnInstigatorSocket;

public:

	void SpawnItem(APawn* Instigator, TSubclassOf<AActor> Item);
		
};
