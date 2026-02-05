// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CarriableComponent.generated.h"

class UHolderComponent;

UCLASS(meta=(BlueprintSpawnableComponent))
class COREGAMEPLAY_API UCarriableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCarriableComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure)
	UHolderComponent* GetHolder() const { return Holder; }

private:

	friend UHolderComponent;
	void SetHolder(UHolderComponent* NewHolder);
	
protected:
	
	UFUNCTION()
	void OnRep_Holder(UHolderComponent* OldHolder);
		
	/**
	 * Override this in blueprint for cosmetic events.
	 */
	UFUNCTION(BlueprintNativeEvent, meta=(ForceAsFunction))
	void OnHolderChanged(UHolderComponent* OldHolder, UHolderComponent* NewHolder);
	
private:

	UPROPERTY(ReplicatedUsing=OnRep_Holder)
	TObjectPtr<UHolderComponent> Holder;
};
