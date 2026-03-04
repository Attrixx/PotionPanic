// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "HolderComponent.generated.h"

UCLASS(meta=(BlueprintSpawnableComponent))
class COREGAMEPLAY_API UHolderComponent : public USphereComponent
{
	GENERATED_BODY()

	UHolderComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void BeginPlay() override;

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
	void Sphere_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_HeldCarriable(TWeakObjectPtr<AActor> OldCarriable);

	/**
	 * Override this for cosmetic events.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnCarriableChanged(AActor* OldCarriable, AActor* NewCarriable);

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCatchAllowed = true;
	
private:

	UPROPERTY(ReplicatedUsing=OnRep_HeldCarriable)
	mutable TWeakObjectPtr<AActor> HeldActor;
};
