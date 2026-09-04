// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Engine/EngineTypes.h"
#include "HolderComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHolderCarriableDelegate, class UHolderComponent*, Holder);

UCLASS(meta=(BlueprintSpawnableComponent))
class COREGAMEPLAY_API UHolderComponent : public USphereComponent
{
	GENERATED_BODY()

	UHolderComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	/**
	 * @returns The held Carriable. If not null, you can assume Implements<UCarriable>() to be true.
	 */
	UFUNCTION(BlueprintPure)
	UObject* GetCarriable() const { return Carriable.Get(); }

	/**
	 * Try to pick up the given Carriable object.
	 * If it is held by another HolderComponent, success depends on whether that component allows stealing.
	 * @return true on success. 
	 */
	UFUNCTION(BlueprintCallable)
	bool TryPickup(UObject* NewCarriable);

	UFUNCTION(BlueprintCallable)
	UObject* Release(FVector Velocity = FVector::ZeroVector);

	/**
	 * Releases the held Carriable and throws it away with EjectForce.
	 * Use it to get rid of an item the holder must not keep.
	 * @returns The ejected Carriable, or null if the holder was empty.
	 */
	UFUNCTION(BlueprintCallable)
	UObject* Eject();

	/**
	 * Drops the held Carriable where it stands, skipping the ground snapping and the physics
	 * Release re-enables. Use it when the Carriable is about to be attached somewhere else, or
	 * outside a game world where physics does not run.
	 * @returns The detached Carriable, or null if the holder was empty.
	 */
	UFUNCTION(BlueprintCallable)
	UObject* Detach();

	/**
	 * Hands the held Carriable over to Target. Neither holder needs bAllowStealing: this is a
	 * hand-over, not a steal.
	 * @returns True on success. On failure nothing moved and the Carriable stays on this holder.
	 */
	UFUNCTION(BlueprintCallable)
	bool TransferTo(UHolderComponent* Target);

	/** Enables or disables catching Carriables that begin overlapping this holder. */
	UFUNCTION(BlueprintCallable)
	void SetCatchAllowed(bool bAllowed) { bIsCatchAllowed = bAllowed; }

	UPROPERTY(BlueprintAssignable)
	FHolderCarriableDelegate OnCarriableChanged;

private:

	UFUNCTION()
	void Sphere_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_Carriable();

	// Applies (bCarried) or reverts (!bCarried) the physics/collision state of a
	// Carriable's primitive. Runs on both authority (from TryPickup/Release) and
	// remote clients (from OnRep_Carriable), since collision profiles and physics
	// simulation state are not replicated.
	void ApplyCarriedState(UObject* InCarriable, bool bCarried);

protected:

	// Can another Holder take from this one?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component")
	uint8 bAllowStealing : 1;

	// Is ICarriable::GetCarriedCollisionProfileName() applied on pick up?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component")
	uint8 bShouldSwitchCollisionProfileOnPickup : 1;

	// Is ICarriable::GetStandaloneCollisionProfileName() applied on release?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component")
	uint8 bShouldSwitchCollisionProfileOnRelease : 1;

	// If Velocity.IsNearlyZero() on release, should we try to snap on the groud and not activate physics?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component")
	uint8 bShouldSnapToGroundOnReleaseWithoutVelocity : 1;

	// Should the holder try to pickup Carriables that begin overlapping with it?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component")
	uint8 bIsCatchAllowed : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component|AttachmentRules")
	EAttachmentRule LocationRule = EAttachmentRule::SnapToTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component|AttachmentRules")
	EAttachmentRule RotationRule = EAttachmentRule::SnapToTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component|AttachmentRules")
	EAttachmentRule ScaleRule = EAttachmentRule::KeepWorld;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0), Category="Holder Component")
	float SnapToGroundMaxDistance = 200.f;

	/**
	 * Launch velocity applied by Eject, in this holder's local space (X forward, Z up).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component")
	FVector EjectForce = FVector(168.f, 0.f, 291.f);

private:

	UPROPERTY(ReplicatedUsing=OnRep_Carriable)
	TWeakObjectPtr<UObject> Carriable;

	// The Carriable whose carried-state we last applied locally. Not replicated:
	// it lets OnRep_Carriable revert the previous Carriable's state on release,
	// without relying on the (unreliable for object refs) RepNotify old-value param.
	TWeakObjectPtr<UObject> LocallyAppliedCarriable;
};
