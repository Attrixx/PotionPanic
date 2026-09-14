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

	/** @returns The held Carriable, local prediction included. Implements<UCarriable>() when not null. */
	UFUNCTION(BlueprintPure)
	UObject* GetCarriable() const { return bHasPrediction ? PredictedCarriable.Get() : Carriable.Get(); }

	/**
	 * Try to pick up the given Carriable object.
	 * If it is held by another HolderComponent, success depends on whether that component allows stealing.
	 * @return true when this machine accepted the pickup, as a decision or as a prediction.
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

	/**
	 * Recomputes InCarriable's physics and collision state from whether anything carries it. Static
	 * so every path that observes a change lands on the same answer, whatever order they run in.
	 */
	UFUNCTION(BlueprintCallable, Category = "Holder Component")
	static void RefreshCarriedState(UObject* InCarriable);

private:

	UFUNCTION()
	void Sphere_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_Carriable();

	/** Attaches InCarriable's primitive to this holder and applies its carried state. */
	void AttachCarriable(UObject* InCarriable);

	/** Detaches InCarriable's primitive, unless something else holds it already. */
	void DetachCarriable(UObject* InCarriable);

	/** Writes the holder's new content: the decision on the authority, a prediction elsewhere. */
	void SetCarriableInternal(UObject* InCarriable);

	/** Drops any standing prediction, putting the server's value back in charge. */
	void ClearPrediction();

	/** Brings the attachment and the carried state in line with GetCarriable(). */
	void ApplyCarriable();

	/** Takes back a prediction the server never answered. */
	void PredictionTimedOut();

	/** @return True where this holder decides its own state rather than receiving it. */
	bool HasHolderAuthority() const;

	/** @return The holder carrying Primitive, found by walking up its attachment. Null when loose. */
	static UHolderComponent* FindCarryingHolder(const UPrimitiveComponent* Primitive);

protected:

	// Can another Holder take from this one?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Holder Component")
	uint8 bAllowStealing : 1;

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

	/**
	 * How long a client's prediction stands without an answer. A refused request changes nothing
	 * server side, so no RepNotify ever comes to take the guess back: keep this above the worst RTT.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0), Category="Holder Component")
	float PredictionTimeoutSeconds = 0.5f;

private:

	/** The server's word. Never written off the authority: that would silence its own RepNotify. */
	UPROPERTY(ReplicatedUsing=OnRep_Carriable)
	TWeakObjectPtr<UObject> Carriable;

	/** What a client believes it holds until the server answers. */
	TWeakObjectPtr<UObject> PredictedCarriable;

	/** True while PredictedCarriable stands in: predicting "nothing" is still a prediction. */
	bool bHasPrediction = false;

	FTimerHandle PredictionTimer;

	// The Carriable whose carried-state we last applied locally. Not replicated:
	// it lets a change revert the previous Carriable's state on release, without
	// relying on the (unreliable for object refs) RepNotify old-value param.
	TWeakObjectPtr<UObject> LocallyAppliedCarriable;
};
