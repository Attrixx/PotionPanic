// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HolderComponent.generated.h"

UCLASS(meta=(BlueprintSpawnableComponent))
class COREGAMEPLAY_API UHolderComponent : public USceneComponent
{
	GENERATED_BODY()

	UHolderComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UFUNCTION(BlueprintPure)
	AActor* GetHeldActor() const;

	UFUNCTION(BlueprintCallable)
	bool TryPickup(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	AActor* Drop();

	UFUNCTION(BlueprintCallable)
	AActor* Throw(FVector Velocity);

protected:

	UFUNCTION()
	void OnRep_HeldCarriable(TWeakObjectPtr<AActor> OldCarriable);

	/**
	 * Override this for cosmetic events.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnCarriableChanged(AActor* OldCarriable, AActor* NewCarriable);

private:

	UPROPERTY(ReplicatedUsing=OnRep_HeldCarriable)
	mutable TWeakObjectPtr<AActor> HeldActor;
};
