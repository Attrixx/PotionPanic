// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "HolderComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCarriableChangedDelegate, UHolderComponent*, Holder, AActor*, OldCarriable, AActor*, NewCarriable);

/**
 * Holder component is an attachment point for actors implementing the Carriable interface.
 * The attachment strategy is defined by the Carriable.
 * The replication of the attachment is also the Carriable responsibility.
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class COREGAMEPLAY_API UHolderComponent : public USphereComponent
{
	GENERATED_BODY()

	UHolderComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void BeginPlay() override;

public:

	UFUNCTION(BlueprintPure)
	AActor* GetHeldActor() const { return HeldActor.Get(); }

	/**
	 * Try to pick up an actor.
	 * This can fail if this Holder already holds something, if the actor doesn't
	 * implement the Carriable interface, or for any reason the Carriable itself
	 * refuses to be picked up.
	 * 
	 * @param Actor Actor to try and pickup.
	 * @param bIsTransferAllowedOnFailure TryTransfer from the actor's Holder on failure?
	 * @return Whether the pickup was successful.
	 */
	UFUNCTION(BlueprintCallable)
	bool TryPickup(AActor* Actor, bool bIsTransferAllowedOnFailure = true);

	/**
	 * Try to transfer the held Carriable to the given Holder.
	 * This can fail if transfer is disallowed on this Holder, if this Holder doesn't hold
	 * something, if the given Holder already holds something, or any reason the Carriable
	 * itself refuses to transfer.
	 * 
	 * @param Dest New Holder for our Carriable.
	 * @return Whether the transfer was successful.
	 */
	UFUNCTION(BlueprintCallable)
	bool TryTransfer(UHolderComponent* Dest);

	UFUNCTION(BlueprintCallable)
	AActor* Drop();
	
	UFUNCTION(BlueprintCallable)
	AActor* Throw(FVector Velocity);

protected:

	UFUNCTION()
	void Sphere_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_HeldActor(TWeakObjectPtr<AActor> OldActor);

public:
	
	// Try catch upon Carriable overlap?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsCatchAllowed : 1;
	
	// TryTransfer from the actor's Holder on failed catch?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsTransferAllowedOnCatchFailure : 1;
	
	// Is transfer FROM this component allowed?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsTransferAllowed : 1;
	
	// Broadcasted on server and client for changes on the held Carriable
	UPROPERTY(BlueprintAssignable)
	FOnCarriableChangedDelegate OnCarriableChanged;
	
private:

	UPROPERTY(ReplicatedUsing=OnRep_HeldActor)
	TWeakObjectPtr<AActor> HeldActor;
};
