// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeliveryComponent.generated.h"

class USocketComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POTIONPANIC_API UDeliveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeliveryComponent();

protected:
	void BeginPlay() override;
	TObjectPtr<USocketComponent> GetSpawnSocket(AActor* TargetActor);

public:

	void Deliver(APawn* Instigator, TObjectPtr<AActor> Item);

};
