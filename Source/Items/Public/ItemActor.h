// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Carriable.h"
#include "GameplayTagContainer.h"
#include "ItemActor.generated.h"

class UItemAsset;
class AItemVisualActor;
class UCapsuleComponent;
class UChildActorComponent;

UCLASS()
class ITEMS_API AItemActor : public AActor, public ICarriable
{
	GENERATED_BODY()

	AItemActor();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void OnConstruction(const FTransform& Transform) override;
	void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	               FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

public:

	UFUNCTION(BlueprintCallable)
	void SetItemAsset(UItemAsset* NewItemAsset);

	/** @return The asset this item currently is, identifying it against recipes and orders. */
	UFUNCTION(BlueprintCallable)
	UItemAsset* GetItemAsset() const { return ItemAsset; }

	UFUNCTION(BlueprintCallable)
	const FGameplayTagContainer& GetItemTags() const { return ItemTags; }

	UFUNCTION(BlueprintCallable)
	void SetItemTags(const FGameplayTagContainer& NewItemTags);

	UFUNCTION(BlueprintCallable)
	void AppendItemTags(const FGameplayTagContainer& NewItemTags);

	UFUNCTION(BlueprintCallable)
	void RemoveItemTag(const FGameplayTagContainer& ItemTagsToRemove);

	/** @return Whether this item breaks on a hard enough impact, read from its runtime tags. */
	UFUNCTION(BlueprintPure)
	bool CanBreak() const;

protected: // ICarriable

	UPrimitiveComponent* GetPrimitive_Implementation() const override;
	FName GetStandaloneCollisionProfileName_Implementation() const override;
	FName GetCarriedCollisionProfileName_Implementation() const override;
	bool CanBeThrown_Implementation() const override;
	void OnThrow_Implementation(FVector Velocity) override;

private:

	// Release and the physics that raises NotifyHit both run on the authority alone, so the
	// cosmetic side of a throw or a break has to be sent rather than derived on each machine.
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Thrown(FVector Velocity);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Broken();

	UFUNCTION()
	void OnRep_ItemAsset();

	UFUNCTION()
	void OnRep_ItemTags();

	void ApplyItemAsset();

	/** Hands ItemTags to the visual. Called from every path that writes them. */
	void NotifyItemTagsChanged();

	/** @return The spawned visual, or null while no item asset has been applied yet. */
	AItemVisualActor* GetVisual() const;

protected:

	/**
	 * Collision and physics body, never rendered. Always a capsule; the visual actor only sizes
	 * it. What the player sees is the visual actor itself.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCapsuleComponent> Body;

	/** Spawns and swaps the item's visual representation (AItemVisualActor) from the ItemAsset. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UChildActorComponent> VisualActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carriable")
	FName StandaloneCollisionProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carriable")
	FName CarriedCollisionProfileName;

private:

	UPROPERTY(EditInstanceOnly, ReplicatedUsing=OnRep_ItemAsset)
	TObjectPtr<UItemAsset> ItemAsset;

	/**
	 * Replicated: an activity can edit these along the way, and clients read them to decide whether
	 * a station is interactable, so deriving them from the asset alone is not enough.
	 */
	UPROPERTY(ReplicatedUsing=OnRep_ItemTags)
	FGameplayTagContainer ItemTags;

	/** Last tags handed to the visual, so an update that changes nothing stays quiet. */
	FGameplayTagContainer LastNotifiedTags;

	/** Keeps a breakable item from firing its break effects again on every following impact. */
	bool bBroken = false;
};
