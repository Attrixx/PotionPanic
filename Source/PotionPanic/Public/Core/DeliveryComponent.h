// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeliveryComponent.generated.h"

class USocketComponent;
class AOrderClient;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POTIONPANIC_API UDeliveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeliveryComponent();

protected:
	void BeginPlay() override;
	TObjectPtr<USocketComponent> GetSocket(AActor* TargetActor);
	TObjectPtr<AOrderClient> ResolveOrderClient() const;

public:

	void Deliver(APawn* Instigator, TObjectPtr<AActor> Item);

private:
	UPROPERTY(EditAnywhere, Category = "Order")
	TObjectPtr<AOrderClient> LinkedOrderClient;

	UPROPERTY()
	TObjectPtr<AOrderClient> CachedOrderClient;
};
