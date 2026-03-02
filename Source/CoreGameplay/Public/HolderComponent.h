// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HolderComponent.generated.h"

class UCarriableComponent;	

UCLASS(meta=(BlueprintSpawnableComponent))
class COREGAMEPLAY_API UHolderComponent : public USceneComponent
{
	GENERATED_BODY()

	UHolderComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UFUNCTION(BlueprintPure)
	UCarriableComponent* GetCarriable() const { return Carriable; }
	
	/**
	 * Replace the CarriableComponent currently held by this component.
	 * @param NewCarriable Can be nullptr.
	 * @return Old Carriable (or nullptr is there was none) 
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UCarriableComponent* Replace(UCarriableComponent* NewCarriable);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void Throw();

protected:
	
	UFUNCTION()
	void OnRep_Carriable(UCarriableComponent* OldCarriable);

	/**
	 * Override this in blueprint for cosmetic events.
	 */
	UFUNCTION(BlueprintNativeEvent, meta=(ForceAsFunction))
	void OnCarriableChanged(UCarriableComponent* OldCarriable, UCarriableComponent* NewCarriable);
	
private:

	UPROPERTY(Replicated)
	TObjectPtr<UCarriableComponent> Carriable;
};
