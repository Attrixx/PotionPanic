// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CarriableComponent.generated.h"

class UHolderComponent;
class UProjectileMovementComponent;

UCLASS(meta=(BlueprintSpawnableComponent))
class COREGAMEPLAY_API UCarriableComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	UCarriableComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void BeginPlay() override;

public:

	UFUNCTION(BlueprintPure)
	UHolderComponent* GetHolder() const { return Holder; }

	UFUNCTION(BlueprintPure)
	bool CanBeThrown() const { return RootPrimitive.IsValid(); }

	UFUNCTION(BlueprintCallable)
	void Throw(const FVector& Impulse);

	UFUNCTION(BlueprintCallable)
	void SnapToGround();

private:

	friend UHolderComponent;
	// Private, called via `UHolderComponent::Replace`
	void SetHolder(UHolderComponent* NewHolder);

protected:

	UFUNCTION()
	void OnRep_Holder(UHolderComponent* OldHolder);

	/**
	 * Override this in blueprint for cosmetic events.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnHolderChanged(UHolderComponent* OldHolder, UHolderComponent* NewHolder);

private:

	UFUNCTION()
	void OnRootPrimitiveHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:

	UPROPERTY(ReplicatedUsing=OnRep_Holder)
	TObjectPtr<UHolderComponent> Holder;

	TWeakObjectPtr<UPrimitiveComponent> RootPrimitive;
};
