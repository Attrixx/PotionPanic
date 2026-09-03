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
};
